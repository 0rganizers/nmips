project('nmips', ['cpp', 'c'],
  version : '0.1',
  default_options : ['warning_level=1', 'c_std=c17'])

ida_sdk = get_option('idasdk')

fs = import('fs')
ida_usr = fs.expanduser('~/.idapro/')

sdk_inc = ida_sdk / 'include'
sdk_bin = ida_sdk / 'bin'
sdk_lib = ida_sdk / 'lib'
plugins = ida_usr / 'plugins'
sdk_ldr = ida_sdk / 'ldr'

# ida_bin = '/Applications/IDA Pro 7.6/ida.app/Contents/MacOS'

# TODO: Hardcoded path
hexrays_sdk = get_option('hexrays_sdk')
hexrays_include = hexrays_sdk / 'include'

summary({
  'sdk': ida_sdk,
  'include': sdk_inc,
  'bin': sdk_bin,
  'lib': sdk_lib,
  'user': ida_usr,
  'plugins': plugins,
  'hexrays': hexrays_sdk
}, section: 'IDA Paths')

src_files = files(
  'loguru.hpp',
  'loguru.cpp',
  'proc_def_shim.cpp',
  'reg.hpp',
  'reg.cpp',
  'elf_ldr.hpp',
  'elf_ldr.cpp',
  'ins.hpp',
  'ins.cpp',
  'ana.cpp',
  'emu.cpp',
  'mopt.hpp',
  'mopt.cpp',
  'mgen.hpp',
  'mgen.cpp',
  'nmips.hpp',
  'nmips.cpp',
  'log.hpp',
  'nanomips-dis.h',
  'nanomips-dis.c',
  'gdb.hpp',
  'gdb.cpp',

  #fuck you binutils
  'binutils/nanomips-opc.c',
  'binutils/pls.c'
)

includes = [sdk_inc, sdk_ldr]
if hexrays_sdk != ''
  includes += [hexrays_include]
endif
sdk_inc_dir = include_directories(includes, is_system: true)

ida_cpu = 'x64'
ida_os = 'linux'
ida_compiler = 'gcc'
ida_bits = '32'
ida_define = '__LINUX__'

if host_machine.cpu_family() == 'aarch64'
  ida_cpu = 'arm64'
endif

if host_machine.system() == 'darwin'
  ida_os = 'mac'
  ida_compiler = 'clang'
  ida_define = '__MAC__'
elif host_machine.system() == 'windows'
  ida_os = 'win'
  ida_compiler = 'vc'
  ida_define = '__NT__'
endif

ida_str = '@0@_@1@_@2@_@3@'.format(ida_cpu, ida_os, ida_compiler, ida_bits)
actual_lib_path = sdk_lib / ida_str
ida_dep = declare_dependency(
  link_args: ['-L'+actual_lib_path, '-lida'],
  include_directories: sdk_inc_dir,
  is_system: true
)

summary({
  'target': ida_str,
  # 'include': sdk_inc_dir,
  'lib': actual_lib_path,
}, section: 'ida')

# elf_lib_path = ida_bin / 'loaders'

# elf_dep = declare_dependency(
#   link_args: ['-L'+elf_lib_path, '-lelf']
# )

# TODO make this generic / build opcodes with meson (at least partially)
# host = 'x86_64-apple-darwin19.6.0'
# target = 'nanomips-gnu-elf'
# src_dir = meson.current_source_dir()
# binutils_prefix = '..' / 'libs' / host / target
# binutils_lib = src_dir / binutils_prefix / 'lib'
# binutils_include = binutils_prefix / 'include'

# summary({
#   'host': host,
#   'target': target,
#   'prefix': binutils_prefix,
#   'include': binutils_include,
#   'lib': binutils_lib
# }, section: 'binutils')

# opcodes = declare_dependency(
#   link_args: ['-L'+binutils_lib, '-lbfd', '-lopcodes'],
#   include_directories: binutils_include,
#   is_system: true
# )

project_args = ['-D@0@=1'.format(ida_define)]

if host_machine.system() != 'windows'
  project_args += [
    '-Wno-nullability-completeness',
    '-Wno-gnu-zero-variadic-macro-arguments',
    '-Wno-unused-variable',
  ]
endif



override_options = []

if host_machine.system() == 'windows'
  override_options += ['cpp_std=c++latest'] # because apparently github actions does not support c++20??
else
  override_options += ['cpp_std=c++20'] # fuck meson
endif

summary({
  'project_args': project_args,
  'override_options': override_options
}, section: 'options')

add_project_arguments(project_args, language: 'cpp')
add_project_arguments(project_args, language: 'c')

thread_dep = dependency('threads')
if host_machine.system() == 'windows'
    dl_dep = dependency('', required: false)
else
    dl_dep = meson.get_compiler('c').find_library('dl', required: true)
endif

inc_dir = include_directories('../binutils/include')
shared_library('nmips', src_files, install: true, install_dir: plugins, dependencies: [ida_dep, thread_dep, dl_dep], include_directories: inc_dir, override_options: override_options)

if host_machine.system() == 'darwin'
  actual_lib_path_arm = sdk_lib / 'arm64_mac_clang_32'
  ida_dep_arm = declare_dependency(
    link_args: ['-L'+actual_lib_path_arm, '-lida'],
    include_directories: sdk_inc_dir,
    is_system: true
  )
  shared_library('arm_nmips', src_files, dependencies: [ida_dep_arm, thread_dep, dl_dep], override_options: override_options, include_directories: inc_dir, link_args: ['-target', 'arm64-apple-macos11'], cpp_args: ['-target', 'arm64-apple-macos11'], c_args: ['-target', 'arm64-apple-macos11'])
endif