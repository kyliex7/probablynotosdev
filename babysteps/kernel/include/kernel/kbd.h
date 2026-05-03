#pragma once
#include <stdint.h>
#include <stddef.h>

void kbd_init(void);
char kbd_getchar(void);
void kbd_readline(char *buf, size_t max);
