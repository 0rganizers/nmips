import idaapi
import ida_dbg
import sys
import ida_kernwin
import ida_segment
import ida_loader
import idc

def main():
    sys.stdout = _orig_stdout
    print("[*] executing startup script...")
    print("[*] loading gdb debugger...")
    plug: idaapi.plugin_t = ida_loader.find_plugin("gdb_user")
    print(f"Plugin: {plug}")
    plug
    res = ida_dbg.load_debugger("gdb", True)
    if res:
        print("[*] successfully loaded gdb debugger")
    else:
        print("[!] failed to load gdb debugger")
        return
    # ida_segment.rebase_program(0x76fdf000, idaapi.MSF_FIXONCE)
    ida_dbg.set_remote_debugger("10.10.20.30", "", 1234)
    ret = ida_dbg.attach_process(0)
    print(f"attached with: {ret}")
print("hello there!")
idc.auto_wait()
print("analysis finished")
# main()