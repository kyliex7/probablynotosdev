#!/usr/bin/env bash
set -euo pipefail

ROOT="$(pwd)"
BACKUP="${ROOT}.bak.$(date +%Y%m%d_%H%M%S)"
echo "Backing up project to: $BACKUP"
cp -a "$ROOT" "$BACKUP"

echo "Creating target layout under kernel/ ..."
mkdir -p kernel/include/kernel
mkdir -p kernel/src/kernel
mkdir -p kernel/src/libc
mkdir -p bin

echo "Moving headers (if present) to kernel/include ..."
# Move known headers if they exist (safe: only move)

[ -f libc/include/string.h ] && mv -n libc/include/string.h kernel/include/ || true
[ -f kernel/kernel/include/kernel/tty.h ] && mv -n kernel/kernel/include/kernel/tty.h kernel/include/kernel/ || true
[ -f kernel/tty.h ] && mv -n kernel/tty.h kernel/include/ || true
[ -f kernel/include/string.h ] && mv -n kernel/include/string.h kernel/include/ || true
# Common minimal headers — move if found

for h in stddef.h stdint.h stdio.h string.h; do
if [ -f "$h" ]; then mv -n "$h" kernel/include/ || true; fi
if [ -f "kernel/$h" ]; then mv -n "kernel/$h" kernel/include/ || true; fi
done

echo "Moving source files into kernel/src ..."
[ -f kernel/kernel/src/kernel/kernel.c ] || true
# Move kernel.c if present in known locations

if [ -f kernel/kernel/kernel.c ]; then
mv -n kernel/kernel/kernel.c kernel/src/kernel/kernel.c
fi
if [ -f kernel/src/kernel/kernel.c ]; then
mv -n kernel/src/kernel/kernel.c kernel/src/kernel/kernel.c || true
fi
# Move libc implementation files if present

for f in memcpy.c memset.c string.c; do
if [ -f libc/src/$f ]; then
mv -n libc/src/$f kernel/src/libc/$f
fi
if [ -f kernel/kernel/src/libc/$f ]; then
mv -n kernel/kernel/src/libc/$f kernel/src/libc/$f || true
fi
if [ -f $f ]; then
mv -n $f kernel/src/libc/$f || true
fi
done

echo "Ensuring arch code stays under arch/ ..."
mkdir -p arch/i386
if [ -f arch/i386/tty.c ]; then
mv -n arch/i386/tty.c arch/i386/tty.c || true
fi

echo "Writing updated Makefile (saved original as makefile.orig if present) ..."
if [ -f makefile ]; then cp -n makefile makefile.orig || true; fi

cat > makefile <<'MAKE'
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

bin/boot.bin: boot.asm
$(NASM) $(NASMFLAGS_BIN) boot.asm -o bin/boot.bin

bin/stage2.o: stage2.asm
$(NASM) $(NASMFLAGS_ELF) stage2.asm -o bin/stage2.o

bin/kernel.o: kernel/src/kernel/kernel.c kernel/include/kernel/tty.h
$(CC) -c kernel/src/kernel/kernel.c -o bin/kernel.o $(CFLAGS)

bin/memcpy.o: kernel/src/libc/memcpy.c kernel/include/string.h
$(CC) -c kernel/src/libc/memcpy.c -o bin/memcpy.o $(CFLAGS)

bin/memset.o: kernel/src/libc/memset.c kernel/include/string.h
$(CC) -c kernel/src/libc/memset.c -o bin/memset.o $(CFLAGS)

bin/string.o: kernel/src/libc/string.c kernel/include/string.h
$(CC) -c kernel/src/libc/string.c -o bin/string.o $(CFLAGS)

bin/stage2.bin: bin/stage2.o bin/kernel.o bin/memcpy.o bin/memset.o bin/string.o
$(LD) $(LDFLAGS) -o bin/stage2.bin \
bin/stage2.o bin/kernel.o bin/memcpy.o bin/memset.o bin/string.o

bin/image.bin: bin/boot.bin bin/stage2.bin
cat bin/boot.bin bin/stage2.bin > bin/image.bin

run: bin/image.bin
qemu-system-x86_64 -drive format=raw,file=bin/image.bin,index=0,if=floppy -monitor stdio

debug: bin/image.bin
qemu-system-x86_64 -drive format=raw,file=bin/image.bin -s -S &
gdb -q -x ~/dotfiles/gdb/x_og.cfg \
-ex "target remote :1234" \
-ex "break *0x7c00" \
-ex "break *0x8000" \
-ex "continue"

clean:
rm -f bin/*
MAKE

echo "Fixing include paths in source files (lightweight) ..."
# Ensure sources include headers with angle brackets pointing to kernel/include
# Replace possible includes like #include "string.h" to <string.h> (only in .c/.h under kernel/)

grep -IlR '#include "string.h"' kernel || true
sed -i 's/#include "string.h"/#include <string.h>/g' $(grep -IlR '#include "string.h"' kernel || true) || true
sed -i 's/#include "stdint.h"/#include <stdint.h>/g' $(grep -IlR '#include "stdint.h"' kernel || true) || true
sed -i 's/#include "stddef.h"/#include <stddef.h>/g' $(grep -IlR '#include "stddef.h"' kernel || true) || true

echo "Done. Backup located at: $BACKUP"
echo "Next steps:"
echo "  1) Inspect the backup if anything looks off."
echo "  2) Ensure toolchain is on PATH or run: export PATH="$ROOT/toolchain/bin:$PATH""
echo "  3) Run: make clean && make run"
echo ""
echo "If build errors show missing includes, open the failing source and add the required #include lines (e.g., <stddef.h>, <stdint.h>, <string.h>, or <kernel/tty.h>)"
