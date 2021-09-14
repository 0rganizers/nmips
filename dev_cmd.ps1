function MsDevShell {
    # Use `vswhere` to locate Visual Studio editions.
    $products = 'Community','Professional','Enterprise','BuildTools' | %{ "Microsoft.VisualStudio.Product.$_" }
    $vswhere = Get-Command 'vswhere'
    $vs = & $vswhere.Path -products $products -latest -version '[16.11,16.12)' -format json | ConvertFrom-Json
    $tools = Join-Path $vs.installationPath 'Common7' 'Tools'
    try {
    # Attempt 1 (Visual Studio 2019 and newer)
    #
    # Look for DevShell.dll and import it. Then use the provided
    # `Enter-VsDevShell` command to merge the DevShell environment into
    # the current environment.
    # Locate DevShell.dll within the Visual Studio installation.
    $devshell = Join-Path $tools 'Microsoft.VisualStudio.DevShell.dll'
    if (!(Test-Path $devshell -Type Leaf)) {
        $devshell = Join-Path $tools 'vsdevshell' 'Microsoft.VisualStudio.DevShell.dll'
    }
    if (!(Test-Path $devshell -Type Leaf)) {
        throw "error: cannot find Microsoft.VisualStudio.DevShell.dll"
    }
    # Import DevShell.dll and use Enter-VsDevShell.
    Import-Module $devshell
    Enter-VsDevShell -VsInstanceId $vs.instanceId -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -no_logo'
    } catch {
        # Print exception for debugging.
        echo $_
        # Attempt 2 (Visual Studio 2017)
        #
        # Execute VsDevCmd.bat and parse its output into a collection. Then
        # take each entry and merge it into the environment. The idea was
        # taken from:
        #     https://github.com/microsoft/vswhere/issues/150#issuecomment-485381959
        # Locate VsDevCmd.bat within the Visual Studio installation.
        $devcmd = Join-Path $tools 'VsDevCmd.bat'
        if (!(Test-Path $devcmd -Type Leaf)) {
            throw "error: cannot find VsDevCmd.bat"
        }
        # Run VsDevCmd.bat and parse the output into a collection.
        $cmd = '"{0}" -arch=x64 -no_logo && pwsh -Command "Get-ChildItem env: | Select-Object Name,Value | ConvertTo-Json"' -f $devcmd
        $output = $(& "${env:COMSPEC}" /s /c $cmd)
        if ($LASTEXITCODE -ne 0) {
            throw $output
        }
        # Merge the output into the environment.
        $output | ConvertFrom-Json | %{ Set-Content "env:$($_.Name)" $_.Value }
    }
}
# Enter VsDevShell and collect the environment before and after.
$before = @{}
Get-ChildItem env: | %{ $before.Add($_.Name, $_.Value) }
MsDevShell
$after = @{}
Get-ChildItem env: | %{ $after.Add($_.Name, $_.Value) }
# Calculate environment update.
$diff = $after.GetEnumerator() | where { -not $before.ContainsKey($_.Name) -or $before[$_.Name] -ne $_.Value }
# Print and export environment update.
echo '----------------------------------------'
echo 'Updated Environment'
echo '----------------------------------------'
$diff | Format-List
echo '----------------------------------------'
$diff | %{ echo "$($_.Name)=$($_.Value)" >> $env:GITHUB_ENV }