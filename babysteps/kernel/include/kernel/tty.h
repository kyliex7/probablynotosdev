#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H
#include <stddef.h>
#include <stdint.h>

void term_init(void);
void term_setclr(uint8_t clr);
void term_scroll(void);
void term_putentryat(unsigned char c, uint8_t clr, size_t x, size_t y);
void term_putchar(unsigned char c);
void term_write(const char* s, size_t size);
void term_writestr(const char* s);
void term_clear(void);

#endif
