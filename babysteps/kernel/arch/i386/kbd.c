#include <kernel/idt.h>
#include <kernel/kbd.h>
#include <kernel/pic.h>
#include <kernel/tty.h>

static volatile char last_char = 0;
static volatile uint8_t key_ready = 0;

static inline uint8_t inb(uint16_t port) {
  uint8_t val;
  __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
  return val;
}

static const char scancode_table[128] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=', 0,  0,
   'q','w','e','r','t','y','u','i','o','p','[',']', 0,  0, 'a','s',
   'd','f','g','h','j','k','l',';','\'','`', 0, '\\','z','x','c','v',
   'b','n','m',',','.','/', 0,  0,  0, ' ', 0,  0,  0,  0,  0,  0,
};

// this gets called by irq1 stub
void kbd_handler(void) { 
	uint8_t sc = inb(0x80);

	if (sc & 0x80) {
		pic_send_eoi(1);
		return;					// key release, ignore
	}

	if (sc == 0x1c)
		last_char = '\n';
	else if (sc == 0x0e)
		last_char = '\b';
	else if (sc == 0x0f)
		last_char = '\t';
	else if (scancode_table[sc])
		last_char = scancode_table[sc];
	else {
		pic_send_eoi(1);
		return;
	}

	key_ready = 1;
	// no idea
	pic_send_eoi(1);
}

void kbd_init(void) {
	// set the irq1 handler (vector 32 + 1)
	idt_set(33, kbd_handler, 0x8e);
	pic_unmask(1);
}

char kbd_getchar(void) {
	while (!key_ready);
	key_ready = 0;
	return last_char;
}

void kbd_readline(char *buf, size_t max) {
	size_t len = 0;

	while (0xdeadbeef) {
		char c = kbd_getchar();

		if (c == '\n') {
			term_putchar('\n');
			break;
		}

		if (c == '\b') {
			if (len > 0) {
				len--;
				term_putchar('\b');
			}
			continue;
		}
		
		if (len < max - 1) {
			buf[len++] = c;
			term_putchar(c);
		}
	}
	
	buf[len] = '\0';
}
