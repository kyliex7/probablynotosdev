TOOLCHAIN := $(CURDIR)/toolchain/bin
CC := $(TOOLCHAIN)/i686-elf-gcc
NASM := nasm
LD := ld
AR := ar

CFLAGS := -ffreestanding -O2 -m32 -Ikernel/include -Wa,--32
NASMFLAGS_BIN := -f bin
NASMFLAGS_ELF := -f elf32
LDFLAGS := -m elf_i386 -T kernel/linker.ld --oformat binary

.PHONY: all run debug clean

all: bin/image.bin

######################################################################
bin/boot.bin: boot.asm
	$(NASM) $(NASMFLAGS_BIN) boot.asm -o bin/boot.bin

bin/stage2.o: stage2.asm
	$(NASM) $(NASMFLAGS_ELF) stage2.asm -o bin/stage2.o

bin/kernel.o: kernel/src/kernel/kernel.c
	$(CC) -c kernel/src/kernel/kernel.c -o bin/kernel.o $(CFLAGS)

bin/arch_tty.o: kernel/arch/i386/tty.c kernel/include/kernel/tty.h
	$(CC) -c kernel/arch/i386/tty.c -o bin/arch_tty.o $(CFLAGS)

bin/gdt.o: kernel/arch/i386/gdt.c kernel/include/kernel/gdt.h
	$(CC) -c kernel/arch/i386/gdt.c -o bin/gdt.o $(CFLAGS)

bin/gdt_asm.o: kernel/arch/i386/gdt.asm
	$(NASM) $(NASMFLAGS_ELF) kernel/arch/i386/gdt.asm -o bin/gdt_asm.o

bin/idt.o: kernel/arch/i386/idt.c kernel/include/kernel/idt.h
	$(CC) -c kernel/arch/i386/idt.c -o bin/idt.o $(CFLAGS)

bin/idt_asm.o: kernel/arch/i386/idt.asm
	$(NASM) $(NASMFLAGS_ELF) kernel/arch/i386/idt.asm -o bin/idt_asm.o

bin/pic.o: kernel/arch/i386/pic.c kernel/include/kernel/pic.h
	$(CC) -c kernel/arch/i386/pic.c -o bin/pic.o $(CFLAGS)

bin/kbd.o: kernel/arch/i386/kbd.c kernel/include/kernel/kbd.h
	$(CC) -c kernel/arch/i386/kbd.c -o bin/kbd.o $(CFLAGS)

########################### LIBC ###########################
bin/memcpy.o: kernel/src/libc/memcpy.c kernel/include/string.h
	$(CC) -c kernel/src/libc/memcpy.c -o bin/memcpy.o $(CFLAGS)

bin/memset.o: kernel/src/libc/memset.c kernel/include/string.h
	$(CC) -c kernel/src/libc/memset.c -o bin/memset.o $(CFLAGS)

bin/string.o: kernel/src/libc/string.c kernel/include/string.h
	$(CC) -c kernel/src/libc/string.c -o bin/string.o $(CFLAGS)

###########################################################
bin/stage2.bin: bin/stage2.o bin/kernel.o bin/arch_tty.o bin/gdt_asm.o bin/gdt.o bin/idt_asm.o bin/idt.o bin/pic.o bin/kbd.o bin/memcpy.o bin/memset.o bin/string.o
	$(LD) $(LDFLAGS) -o bin/stage2.bin \
	bin/stage2.o bin/kernel.o bin/arch_tty.o bin/gdt.o bin/gdt_asm.o bin/idt.o bin/idt_asm.o bin/pic.o bin/kbd.o bin/memcpy.o bin/memset.o bin/string.o

bin/image.bin: bin/boot.bin bin/stage2.bin
	cat bin/boot.bin bin/stage2.bin > bin/image.bin

run: bin/image.bin
	qemu-system-x86_64 -drive format=raw,file=bin/image.bin,index=0,if=floppy

debug: bin/image.bin
	qemu-system-x86_64 -drive format=raw,file=bin/image.bin -s -S &
	gdb -q -x ~/dotfiles/gdb/x_og.cfg \
	-ex "target remote :1234" \
	-ex "break *0x7c00" \
	-ex "break *0x8000" \
	-ex "continue"

clean:
	rm -f bin/*
