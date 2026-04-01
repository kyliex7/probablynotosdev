gdt_start:
  dq 0x00             ;; the null descriptor

gdt_code:
  dw 0xffff           ;; limit
  dw 0x00             ;; base
  db 0x00             ;; base
  db 10011010b        ;; access byte
  db 11001111b        ;; flags + limit
  db 0x00             ;; base

gdt_data:
  dw 0xffff           ;; limit
  dw 0x00             ;; base
  db 0x00             ;; base
  db 10010010b        ;; access byte
  db 11001111b        ;; flags + limit
  db 0x00             ;; base

gdt_end:

;; for lgdt
gdt_descriptor:
  dw gdt_end - gdt_start - 1  ;; sizeof(gdt)
  dd gdt_start                ;; *gdt
