[ORG 0x7c00]
bits 16

loader:
  cli

  ;; seg init
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov sp, ax
  mov ss, ax            ;; 0x7c00 * 16 = 0x7c00 <- where we want the ss to be placed

  ;; sp init
  mov ax, 0x7c00        ;; 2-KB stack
  mov sp, ax
  mov bp, sp

  sti

  cld
  mov si, msg1
  call bios_print
  mov si, msg2
  call bios_print
  mov si, msg3
  call bios_print

  call load_stage_two
  jmp 0x8000

load_stage_two:
  xor ax, ax               ;; zero out es cus,
  mov es, ax               ;; absaddr = (seg * 16) + offset
  mov ah, 0x02             ;; read sectors
  mov al, 0x01             ;; number of sectors to read
  mov ch, 0x00             ;; cylinder number
  mov cl, 0x02             ;; sector number to read
  mov dh, 0x00             ;; head number
  ;; let the BIOS to decide where to read from
  ;; BIOS sets the dl to the id of the bootable hard drive
  ;; after the bootloader is loaded into the memory
  mov dl, 0x00             ;; 0x81/0x80 to read from hard drive, 0x00 for the floppy
  mov bx, 0x8000           ;; location we want to load the data (offset to absaddr formula)
  int 0x13
  jc disk_err
  ret

disk_err:
  mov si, loaderr
  call bios_print
  cli
  hlt
  jmp $

%include "include/bios_print.inc"

msg1: db 0xa, 0
msg2: db "here we go bruh", 0xd, 0xa, 0
msg3: db "[+] successfully loaded the bootloader", 0xd, 0xa, 0
loaderr: db "[-] failed loading the code from disk", 0x0d, 0x0a, 0

times 510 - ($ - $$) db 0
dw 0xaa55
