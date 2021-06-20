project('nmips', ['cpp', 'c'],
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

ida_sdk = get_option('idasdk')

fs = import('fs')
ida_usr = fs.expanduser('~/.idapro/')

sdk_inc = ida_sdk / 'include'
sdk_bin = ida_sdk / 'bin'
sdk_lib = ida_sdk / 'lib'
plugins = ida_usr / 'plugins'

summary({
  'sdk': ida_sdk,
  'include': sdk_inc,
  'bin': sdk_bin,
  'lib': sdk_lib,
  'user': ida_usr,
  'plugins': plugins
}, section: 'IDA Paths')

src_files = files(
  'ana.cpp',
  'nmips.hpp',
  'nmips.cpp',
  'log.hpp',
  'nanomips-dis.h',
  'nanomips-dis.c'
)

sdk_inc_dir = include_directories(sdk_inc)

# TODO make this generic
actual_lib_path = sdk_lib / 'x64_mac_clang_32'
ida_dep = declare_dependency(
  link_args: ['-L'+actual_lib_path, '-lida'],
  include_directories: sdk_inc_dir
)

# TODO make this generic / build opcodes with meson (at least partially)
host = 'x86_64-apple-darwin19.6.0'
target = 'nanomips-gnu-elf'
src_dir = meson.current_source_dir()
binutils_prefix = '..' / 'libs' / host / target
binutils_lib = src_dir / binutils_prefix / 'lib'
binutils_include = binutils_prefix / 'include'

summary({
  'host': host,
  'target': target,
  'prefix': binutils_prefix,
  'include': binutils_include,
  'lib': binutils_lib
}, section: 'binutils')

opcodes = declare_dependency(
  link_args: ['-L'+binutils_lib, '-lbfd', '-lopcodes'],
  include_directories: binutils_include
)

# TODO make this work for non macOS
extra_defines = ['__MAC__']
shared_library('nmips', src_files, install: true, install_dir: plugins, dependencies: [ida_dep, opcodes])