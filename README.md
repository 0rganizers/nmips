# nmips

IDA plugin for nanoMIPS. Tested on IDA 7.6.

To see how well it works, with the mipscoder binary from 0CTF, see below :)
You can disassemble, decompile and even debug it!

![mipscoder main decompiled](docs/debugging.png)
![mipscoder rotate switch](docs/switch.png)
![mipscoder start disassembled](docs/asm.png)

## Functionality

Currently, the following works:

- debugging (needs some messing around with configs and relocations currently)
- creating relocations for libraries (e.g. libc)
- decompiling and disassembling (not all instructions are currently implemented)
- custom hexrays optimizer to fix stack variables being messed up
- automatic switch statement detection
- more stuff I probably forgot


## Implementation

The basic idea behind the plugin is, to still load the binary with the MIPS processor module.
The plugin registers a bunch of plugin hooks, so that it can then give IDA the illusion of working on a "normal" MIPS binary. To that end, the binary translates any nanoMIPS instruction into the equivalent MIPS version, or - if it does not exist - implement it itself.

In case the instruction is translated to MIPS, it will be decompiled automatically without any issues.
Otherwise, decompiler hooks emit the correct hexrays microcode, so that these instructions can also be decompiled correctly.

## Installation

| OS | Download |
|----|----------|
|Linux| [Download](https://nightly.link/0rganizers/nmips/workflows/main/main/nmips_linux.zip)|
|macOS (ARM might be broken) | [Download](https://nightly.link/0rganizers/nmips/workflows/main/main/nmips_macos.zip)|
|Windows| [Download](https://nightly.link/0rganizers/nmips/workflows/main/main/nmips_windows.zip)|

Download the corresponding version for your OS and put the plugin inside `~/.idapro/plugins`.
Done! If you open a nanoMIPS ELF file, you should be able to just mash through some of the dialogs and get it working.
If you want to e.g. apply this to a flat binary file, you can instead just load it as a little endian MIPS file.
Then, select this plugin from `Edit > Plugins > nmips`.
This will force it on, and it should start to disassemble stuff!

## Building

Make sure you have meson installed.
Then inside the `plugin` directory, just run:

```bash
meson setup builddir -Didasdk=$IDA_SDK
meson compile -C builddir
```

## TODOs

- implement assembler
- fix debugging to be nicer
- rework plugin to be nicer