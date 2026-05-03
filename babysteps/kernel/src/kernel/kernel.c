#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/kbd.h>
#include <kernel/pic.h>
#include <kernel/tty.h>
#include <string.h>

static int dbg = 0;

void kmain(void) {
  gdt_init();
  idt_init();
  pic_init(IRQ_BASE, IRQ_BASE + 8);

  // unmask the keyboard irq
  pic_unmask(1);
  kbd_init();

  __asm__ volatile("sti");

  term_init();

  if (dbg) {
    term_writestr("[+] gdt loaded\n");
    term_writestr("[+] idt loaded\n");
    term_writestr("[+] interrupts live\n");
  }

  term_writestr("Welcome to the CatOS v0.0\n");
	// todo: fix kbd

	for (;;);
}
