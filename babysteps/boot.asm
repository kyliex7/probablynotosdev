[ORG 0x7c00]
bits 16

loader:
  cli
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov sp, ax
  mov sp, 0x9000
  mov bp, 0x9000
  sti

  ;; for debugging
  mov ax, 0xdead
  mov bx, 0xbeef
  push ax
  push bx
  pop ax
  pop bx
  ;;

  cld
  mov si, msg
  call bios_print

.hang:
  jmp .hang

bios_print:
  lodsb
  or al, al
  jz .done

  mov ah, 0x0e
  mov bh, 0
  int 0x10
  jmp bios_print
.done:
  ret

msg: db "hello from this side", 0xd, 0xa, 0

times 510 - ($ - $$) db 0
dw 0xaa55
