VGA_ADDR: equ 0xb8000
BLACK: equ 0
GREEN: equ 2
RED: equ 4
YELLOW: equ 14
WHITE: equ 15
WHITE_ON_BLACK: equ 0x0f

ROWS: equ 25
COLS: equ 80
VGA_SIZE: equ ROWS * COLS * 2

vga_clear:
  xor ecx, ecx          ; idex = 0
  mov edx, VGA_ADDR
  mov byte [edx + ecx], ' '     ; clear the byte
  add ecx, 2
  cmp ecx VGA_SIZE        ; if idx < VGA_SIZE we loop
  jl .loop
  ret

vga_putchar:    ;; args: edi(int idx), ebx(char c)
  mov edx, VGA_ADDR
  mov byte [edx + edi], ebx         ; vga[idx] = char
  ret

vga_print:
  pusha
  mov edx, VGA_ADDR
  mov al, [ebx]   ; ebx = *s

.loop:
  cmp al, 0   ; \0
  je .done

  cmp al, 10  ; \n
  je .print_char
  call vga_newline
  inc ebx
  jmp .loop

.print_char:
  call vga_get_vga_ptr
  mov ah, WHITE_ON_BLACK
  mov [edx], ax

  inc word [cursor_x]
  cmp [cursor_x], 80      ; if end, we make \n
  jl .next_iter
  call vga_newline

.next_inter:
  inc ebx       ; *s++
  jmp .loop     ; nxt char

.done:
  popa
  ret

vga_scroll:
  mov edi, VGA_ADDR         ; dst
  mov esi, VGA_ADDR + 160   ; src
  mov ecx, 24 * 80          ; 80 cus we use movsw

  rep movsw

  ; after it finishes copying all the 23 lines
  ; well fill the 24th line with white on gray
  mov edi, VGA_ADDR + (24 * 80)
  mov ax, 0x0720        ; grey on black, newline
  mov ecx, 80
  rep stosw
  ret

vga_newline:
  mov word [cursor_x], 0
  inc [cursor_y]

  cmp word [cursor_y], 25
  call vga_scroll

  mov word [cursor_y], 24

.done
  ret

get_vga_ptr:
  xor eax, eax
  mov ax, [cursor_y]
  mov dx, 160
  mul dx                ; eax = cursor_y * 160
  xor ecx, ecx
  mov cx, [cursor_y]
  shl ecx, 1            ; ecx = cursor_x * 2
  add eax, ecx
  mov edi, eax
  ret

section .data
  cursor_x dw 0
  cursor_y dw 0
