#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15
} vga_color_t;

#define VGA_MEMORY 0xB8000
typedef uint8_t u8;
typedef uint16_t u16;
typedef unsigned char uchar;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

size_t term_row, term_col;
u8 term_clr;
u16* term_buf = (u16*)VGA_MEMORY;

void term_init(void);
void term_setclr(u8 clr);
void term_scroll(void);
void term_putentryat(uchar c, u8 clr, size_t x, size_t y);
void term_putchar(uchar c);
void term_write(const char* s, size_t size);
void term_writestr(const char* s);

void kmain(void) {
  term_init();
  for (int i = 0; i < 25; i++) {
    term_writestr("while 0xdeadbeef\n");
  }
  term_writestr("while 0xcafebabe\n");
}

static inline u8 vga_entry_clr(vga_color_t fg, vga_color_t bg) {
  return fg | bg << 4;
}

static inline u16 vga_entry(uchar c, u8 color) {
  return (u16)c | (u16)color << 8;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (s[len++]);
  return len - 1;
}

void term_init(void) {
  term_row = 0;
  term_col = 0;
  term_clr = vga_entry_clr(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t idx = y * VGA_WIDTH + x;
      term_buf[idx] = vga_entry(' ', term_clr);
    }
  }
}

void term_setclr(u8 clr) { term_clr = clr; }

void term_scroll() {
  // move every row one line up
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      term_buf[y * VGA_WIDTH + x] = term_buf[(y + 1) * VGA_WIDTH + x];
    }
  }

  // clear the last row
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    term_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', term_clr);
  }

  // update the row
  term_row = VGA_HEIGHT - 1;
}

void term_putentryat(uchar c, u8 clr, size_t x, size_t y) {
  const size_t idx = y * VGA_WIDTH + x;
  term_buf[idx] = vga_entry(c, clr);
}

void term_putchar(uchar c) {
  if (c == '\n') {
    term_col = 0;
    if (++term_row >= VGA_HEIGHT) term_scroll();
    return;
  }

  if (c == '\b') {
    if (term_col > 0) {
      term_col--;
      term_putentryat(' ', term_clr, term_col, term_row);
    }
    return;
  }

  if (c == '\t') {
    term_col = (term_col + 4) & ~3;
    if (term_col >= VGA_WIDTH) term_scroll();
    return;
  }

  term_putentryat(c, term_clr, term_col, term_row);

  if (++term_col >= VGA_WIDTH) {
    term_col = 0;
    if (++term_row >= VGA_HEIGHT) {
      term_scroll();
    }
  }
}

void term_write(const char* s, size_t size) {
  for (size_t i = 0; i < size; i++) {
    term_putchar(s[i]);
  }
}

void term_writestr(const char* s) { term_write(s, strlen(s)); }
