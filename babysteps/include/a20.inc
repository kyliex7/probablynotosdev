check_a20:
  ;; save
  push ax

  mov ax, 0x2402
  int 0x15

  jc .err_a20_chk
  test al, al             ;; ah = 00: disabled, 01:enabled
  jz .enable_a20

  ;; restore and get tf out
  pop ax
  ret

.enable_a20:
  push ax

  mov ax, 0x2401
  int 0x15

  jc .err_a20_enable

  call check_a20          ;; let's hope no infinite loop will bang us

  ;; we won't reach here, but in case if it did -- we should have a plan b
  ret

.err_a20_enable:
  mov si, msg_a20_enable_err
  call bios_print
  hlt

.err_a20_chk:
  mov si, msg_a20_chk_err
  call bios_print
  hlt

msg_a20_enable_err: db "[-] unable to enable A20", 0x0d, 0x0a, 0
msg_a20_chk_err: db "[-] unable to check A20 status", 0x0d, 0x0a, 0
