### structure
```
bin
├── arch_tty.o
├── boot.bin
├── gdt_asm.o
├── gdt.o
├── idt_asm.o
├── idt.o
├── image.bin
├── kbd.o
├── kernel.o
├── memcpy.o
├── memset.o
├── pic.o
├── stage2.bin
├── stage2.o
└── string.o
boot.asm
include
├── a20.inc
├── bios_print.inc
├── gdt.inc
├── pm_print.inc
└── vga.inc
kernel
├── arch
│   └── i386
│       ├── gdt.asm
│       ├── gdt.c
│       ├── idt.asm
│       ├── idt.c
│       ├── kbd.c
│       ├── pic.c
│       ├── tty.c
│       └── vga.h
├── include
│   ├── kernel
│   │   ├── gdt.h
│   │   ├── idt.h
│   │   ├── kbd.h
│   │   ├── pic.h
│   │   └── tty.h
│   ├── stdbool.h
│   ├── stddef.h
│   ├── stdint.h
│   ├── stdio.h
│   └── string.h
├── linker.ld
└── src
├── kernel
│   └── kernel.c
└── libc
├── memcpy.c
├── memset.c
└── string.c
links.txt
makefile
roadmap.md
stage2.asm
```
