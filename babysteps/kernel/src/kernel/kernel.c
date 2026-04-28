#include <kernel/tty.h>

void kmain(void) {
  term_init();
	term_writestr("hello, kernel world!\n");
  for (;;);
}
