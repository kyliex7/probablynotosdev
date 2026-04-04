;; bruh i finally chose give up
;; will implement this again in the meantime, but c is kinda cool to write the kernel ngl
;; asm is also, but c is the best as i think
;; idfk, maybe skill issue (it is)
VGA_ADDR: equ 0xb8000
VGA_END:  equ 0xb8000 + (25 * 80 * 2)
WHITE_ON_BLACK: equ 0x0f

vga_clear:
  pusha
  mov edi, VGA_ADDR
  mov ax, 0x0f20          ; white on black space
  mov ecx, 80 * 25
  rep stosw

  ; reset pointer to start
  mov dword [vga_current_ptr], VGA_ADDR
  popa
  ret

vga_putchar:    ;; args: bl(char c)
  push eax
  mov edi, [vga_current_ptr]

  ; chk if we hit the EOS
  cmp edi, VGA_END
  jl .do_put
  call vga_scroll
  mov edi, [vga_current_ptr]

.do_put:
  mov [edi], bl
  mov byte [edi + 1], WHITE_ON_BLACK
  add dword [vga_current_ptr], 2
  pop eax
  ret

vga_print:
  pusha
.loop:
  mov al, [ebx]
  cmp al, 0
  je .done

  cmp al, 10      ; \n
  jne .print_char
  call vga_newline
  inc ebx
  jmp .loop

.print_char:
  mov bl, al
  call vga_putchar

  ; If we just filled the last character of a line,
  ; putchar doesn't know. We check if pointer is at a line boundary.
  mov eax, [vga_current_ptr]
  sub eax, VGA_ADDR
  xor edx, edx
  mov ecx, 160
  div ecx
  test edx, edx   ; If edx is 0, we are at the start of a new line
  jnz .next_iter
  ; Note: we don't call newline here because putchar already moved us
  ; to the start of the next line by adding 2.

.next_iter:
  inc ebx
  jmp .loop

.done:
  popa
  ret

vga_scroll:
  pusha
  cld
  mov edi, VGA_ADDR           ; dst: line 0
  mov esi, VGA_ADDR + 160     ; src: line 1
  mov ecx, (24 * 80)          ; 24 lines, 80 words each
  rep movsw

  ; Clear the last line
  mov edi, VGA_ADDR + (24 * 80 * 2)
  mov ax, 0x0f20
  mov ecx, 80
  rep stosw

  ; Set pointer to the start of the last line
  mov dword [vga_current_ptr], VGA_ADDR + (24 * 80 * 2)
  popa
  ret

vga_newline:
  pusha
  mov eax, [vga_current_ptr]
  sub eax, VGA_ADDR
  xor edx, edx
  mov ecx, 160
  div ecx         ; eax = current row, edx = bytes into current row

  mov ebx, 160
  sub ebx, edx    ; ebx = bytes left in this row
  add dword [vga_current_ptr], ebx

  ; If we just moved past the last line, scroll
  cmp dword [vga_current_ptr], VGA_END
  jl .done
  call vga_scroll

.done:
  popa
  ret

; Use a single pointer instead of X/Y
vga_current_ptr: dd 0xb8000
