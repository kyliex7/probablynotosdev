# kernel.md

> Building a kernel → shell → "go-crazy" animation. That's the whole project.

---

## The end goal, spelled out

```
power on
    │
    ▼
boot.asm          you already have this
    │
    ▼
stage2.asm        you already have this (protected mode switch)
    │
    ▼
kernel.asm / kernel.c
    ├── VGA driver        (write anywhere on screen, colors)
    ├── Keyboard driver   (read what the user types)
    └── Shell             (wait for "go-crazy", then explode)
```

No filesystem. No processes. No memory manager.
Just bare metal → screen output → keyboard input → your shell.
That's enough for the goal.

---

## Phase 1 — VGA driver

**What it is:** direct writes to `0xB8000`. 80 columns × 25 rows, 2 bytes per cell (char + color).

**What you need to write:**

```nasm
; cell index formula
; index = (row * 80 + col) * 2
; byte 0 = ASCII char
; byte 1 = color attribute

; color byte = (bg << 4) | fg
; fg/bg values 0-15:
;   0=black  1=blue    2=green  3=cyan
;   4=red    5=magenta 6=brown  7=light grey
;   8=dark grey        9=light blue
;   10=light green     11=light cyan
;   12=light red       13=light magenta
;   14=yellow          15=white
```

**Functions to write, in order:**

```
vga_clear       wipe whole screen to blank + one color
vga_putchar     write one char at (row, col) with color
vga_print       loop over a string calling vga_putchar
vga_scroll      shift all rows up by one, blank the last row
vga_newline     advance cursor to next row, scroll if needed
```

**Milestone:** your kernel boots and prints your name in a color you chose.
If you can do that, VGA is done.

---

## Phase 2 — Keyboard driver

**What it is:** reading the PS/2 keyboard controller at port `0x60`.

**The easy way (polling, no interrupts):**

```nasm
; wait until keyboard has data, then read it
wait_key:
    in al, 0x64         ; read status port
    test al, 1          ; bit 0 = output buffer full
    jz wait_key         ; loop until key is ready
    in al, 0x60         ; read the scancode
    ret                 ; al = scancode
```

No IDT setup needed. No interrupts. Just spin-wait.
It's not how a real OS does it, but it works perfectly for a shell.

**Scancode → ASCII:**
The keyboard sends _scancodes_, not ASCII. You need a lookup table.

```nasm
; scancode set 1 (what BIOS/legacy PS2 uses)
; key press  = scancode as-is     (0x01 to 0x58)
; key release = scancode | 0x80   (ignore these)

scancode_table:
    db 0,   27,  '1', '2', '3', '4', '5', '6'   ; 0x00-0x07
    db '7', '8', '9', '0', '-', '=', 8,   9     ; 0x08-0x0F  (8=backspace, 9=tab)
    db 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'   ; 0x10-0x17
    db 'o', 'p', '[', ']', 13,  0,   'a', 's'   ; 0x18-0x1F  (13=enter)
    db 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'   ; 0x20-0x27
    db 39,  '`', 0,   92,  'z', 'x', 'c', 'v'   ; 0x28-0x2F  (39=apostrophe)
    db 'b', 'n', 'm', ',', '.', '/', 0,   '*'   ; 0x30-0x37
    db 0,   ' '                                  ; 0x38-0x39  (space)
    ; fill the rest with 0 for now
```

**Functions to write:**

```
kbd_read_scancode    poll port 0x60, return raw scancode
kbd_scancode_to_ascii    look up in table, return 0 if not printable
kbd_getchar         loop until a printable key, return ASCII char
```

**Milestone:** you can type a character and see it appear on screen.

---

## Phase 3 — Shell

**What it is:** a loop that reads characters, builds a string, and checks what was typed.

**The logic (works in both C and assembly):**

```
loop forever:
    print prompt  "$ "
    read chars into buffer until Enter is pressed
    if buffer == "go-crazy":
        run the animation
    else if buffer == "clear":
        clear screen
    else if buffer == "hello":
        print something fun
    else:
        print "unknown command: " + buffer
    clear the buffer
    go back to loop
```

**The input buffer:**

```nasm
input_buf:  times 64 db 0    ; 64 char max input
input_len:  db 0             ; how many chars so far
```

**Reading input:**

```nasm
read_line:
    mov esi, input_buf
    mov byte [input_len], 0
.loop:
    call kbd_getchar          ; get one char (blocks until keypress)
    cmp al, 13                ; Enter key?
    je .done
    cmp al, 8                 ; Backspace?
    je .backspace
    cmp byte [input_len], 63  ; buffer full?
    je .loop
    ; store char, print it, advance cursor
    mov [esi], al
    inc esi
    inc byte [input_len]
    call vga_putchar_at_cursor
    jmp .loop
.backspace:
    cmp byte [input_len], 0
    je .loop                  ; nothing to delete
    dec esi
    dec byte [input_len]
    call vga_erase_at_cursor  ; overwrite with space
    jmp .loop
.done:
    mov byte [esi], 0         ; null terminate
    ret
```

**String compare (you need this for command matching):**

```nasm
; compare [esi] with [edi], return ZF set if equal
strcmp:
    .loop:
        mov al, [esi]
        mov bl, [edi]
        cmp al, bl
        jne .not_equal
        test al, al           ; both zero = end of string
        jz .equal
        inc esi
        inc edi
        jmp .loop
    .equal:
        xor eax, eax          ; ZF set
        ret
    .not_equal:
        mov eax, 1
        ret
```

**Milestone:** you can type "hello" and get a response. Shell is working.

---

## Phase 4 — The go-crazy animation

**What it is:** pure VGA writes in a loop with delay. No OS needed.

**The building blocks:**

```nasm
; burn CPU cycles for ~N milliseconds (tune the constant for your machine/QEMU)
delay:
    mov ecx, 500000     ; tweak this
.loop:
    dec ecx
    jnz .loop
    ret

; write a random-ish char at random-ish position
; (use a simple LCG if you want actual randomness)
; LCG: next = (prev * 1664525 + 1013904223) mod 2^32
```

**Animation ideas, from simple to wild:**

```
Level 1 — color sweep
    loop through every cell, change the color attribute
    cycle through 16 colors with a delay between each pass
    your name stays in the center, everything else cycles

Level 2 — matrix rain
    pick random columns, drop a character down it each frame
    leave a trail that fades (changes color each row lower)
    classic green on black

Level 3 — bouncing name
    store your name as a string
    track x, y position and dx, dy velocity
    each frame: erase old position, update x+y, draw at new position
    bounce off edges (x=0, x=80-len, y=0, y=24)

Level 4 — keygen style
    split screen: top half = scrolling hex-looking garbage
    bottom half = your name assembling letter by letter
    play with colors: each char of your name a different color
    border of cycling symbols around the edge

Level 5 — all of the above
    combine them, add a "press any key to exit" check
    when key pressed: do a dramatic wipe and return to shell
```

**Exit the animation:**

```nasm
animate_loop:
    ; ... draw frame ...
    call delay

    ; non-blocking key check
    in al, 0x64
    test al, 1          ; key waiting?
    jz animate_loop     ; no, keep going
    in al, 0x60         ; consume the scancode
    ret                 ; back to shell
```

---

## Porting your existing C shell

You wrote a shell in C already. Porting it means:

**What changes:**

- `printf` → your `vga_print` function
- `fgets` / `scanf` → your `read_line` function
- `strcmp` → your `strcmp` function
- `system()` / `fork()` / `exec()` — these don't exist, replace with direct function calls

**What stays the same:**

- The command parsing logic
- The string comparison for command names
- The overall loop structure

**The port process:**

1. Copy your shell's main loop logic
2. Replace every stdlib call with your kernel equivalent
3. Compile with `i686-elf-gcc -ffreestanding -nostdlib`
4. Link it into your kernel binary

If your shell is pure C with no file I/O or process management, the port is mostly find-and-replace on function names. If it uses `fork`/`exec`/pipes — strip those out, you don't have processes yet.

---

## File structure for this project

```
osdev/
├── boot.asm          sector 1 — BIOS loads this
├── stage2.asm        sector 2 — GDT + protected mode switch
├── kernel/
│   ├── kernel.asm    entry point, calls shell_main
│   ├── vga.asm       vga_clear, vga_putchar, vga_print, vga_scroll
│   ├── kbd.asm       kbd_read_scancode, kbd_getchar
│   ├── shell.asm     read_line, command dispatch
│   └── crazy.asm     the animation
├── Makefile
└── os.img            output — boot it in QEMU
```

Or if mixing C for the kernel:

```
osdev/
├── boot.asm
├── stage2.asm
├── kernel/
│   ├── entry.asm     just sets up stack, calls kernel_main
│   ├── vga.c         + vga.h
│   ├── kbd.c         + kbd.h
│   ├── shell.c       (your existing shell, ported)
│   └── crazy.c       (animation)
├── linker.ld
└── Makefile
```

---

## Makefile for the whole thing

```makefile
AS      = nasm
CC      = i686-elf-gcc
LD      = i686-elf-gcc

ASFLAGS = -f bin
CFLAGS  = -ffreestanding -O2 -Wall -Wextra -std=gnu99
LDFLAGS = -ffreestanding -nostdlib -lgcc

# pure assembly build
os.img: boot.bin stage2.bin kernel.bin
	cat boot.bin stage2.bin kernel.bin > os.img

boot.bin: boot.asm
	$(AS) -f bin boot.asm -o boot.bin

stage2.bin: stage2.asm
	$(AS) -f bin stage2.asm -o stage2.bin

kernel.bin: kernel/kernel.asm kernel/vga.asm kernel/kbd.asm kernel/shell.asm kernel/crazy.asm
	$(AS) -f bin kernel/kernel.asm -o kernel.bin

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

debug: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img -s -S &
	gdb -ex "target remote :1234" -ex "set arch i386"

clean:
	rm -f *.bin *.img kernel/*.bin
```

---

## Build order to not go insane

Build one thing at a time. Test each piece before moving on.

```
Week 1
    day 1-2:  vga_clear + vga_putchar working, print your name on boot
    day 3-4:  vga_print, vga_scroll, newline handling
    day 5:    full VGA driver done, looks clean

Week 2
    day 1-2:  kbd_getchar working, echo chars to screen
    day 3-4:  read_line with backspace working
    day 5:    type "hello" → shell responds

Week 3
    day 1-3:  port your C shell, all commands working
    day 4-5:  "go-crazy" triggers, basic animation running

Week 4+
    make the animation as wild as you want
    it never has to be "done"
```

---

## Resources for the animation part specifically

- **nanochess/pbsgames** on GitHub — Oscar Toledo's boot sector games.
  Entire games in 512 bytes. The VGA tricks in there are exactly what you need.
  Especially look at how he does color cycling and sprite movement.

- **pouet.net** — demoscene releases. Filter by "wild" category.
  These people have been doing crazy bare-metal animations since the 90s.
  Source code for a lot of them is public.

- **scene.org** — archive of demoscene productions. Free downloads.

- **VGA hardware reference** on OSDev wiki — when you want to go beyond text mode
  into actual pixel graphics (320×200, 256 colors — Mode 13h).
  One day, not now.

---

## The one thing that will save your sanity

When something breaks and the screen goes black — add a debug marker:

```nasm
; drop this anywhere to see "did we reach here?"
mov edi, 0xB8000
mov word [edi], 0x4F58   ; 'X' in white on red — impossible to miss
```

Red X on screen = you got there.
No red X = you crashed before that line.
Binary search your way to the bug.

---

_The whole project is about 600-900 lines of assembly (or ~400 lines of C + 200 assembly).
Totally doable. Start with Phase 1 today._
