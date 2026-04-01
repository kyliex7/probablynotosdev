VIDEOM equ 0xb8000
WOB equ 0x0f           ;; white on black lol

pm_print:
  pusha
  mov edx, VIDEOM

.loop:
  mov al, [ebx]
  mov ah, WOB

  cmp al, 0
  je .done

  mov [edx], ax
  inc ebx
  inc edx
  inc edx

  jmp .loop

.done:
  popa
  ret
