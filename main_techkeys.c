#include "platforms.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "usb_keyboard.h"
#include "io.h"
#include "hid.h"
#include "timer.h"
#include "aux.h"
#include "display.h"
#include "gfx.h"


volatile bool should_scan = false;

int main(void)
{
	clock_prescale_set(clock_div_1);

	sei();
	USB_init();
	HID_commit_state();

	for (int i = 0; i < 4; ++i) {
		IO_set(i, true);
		IO_config(i, OUTPUT);
	}
	IO_config(0, INPUT);
	IO_config(1, INPUT);
	IO_config(2, INPUT);
	IO_config(3, INPUT);
	IO_set(0, true);
	IO_set(1, true);
	IO_set(2, true);
	IO_set(3, true);

	//TIMER_init();

	DISPLAY_init();
	char *txt = "techkeys.us - the best keycaps in the world!";
	int st = 24 + 6*strlen(txt);
	int i = 0;
	int d = 11;
	uint8_t b = 0;
	while (true) {
		int p = (i/3)%9;
		if (p < 5)
			p -= 2;
		else
			p = 7-p;
		uint8_t br = 3;

		GFX_fill((struct rect){0, 0, d, 7}, 1);
		GFX_fill((struct rect){24-d, 0, d, 7}, 1);
		GFX_put_text((struct rect){d, 0, 24 - 2*d, 7}, 24 - i - d, p, txt, br, 0);
		GFX_swap();
		_delay_ms(20);
		if (i % 16 == 0 && d > 0)
			--d;
		++i;
		if (i > st)
			i = 0;
		++b;

		bool k = false;
		for (int i = 0; i < 4; ++i) {
			if (!IO_get(i)) {
				k = true;
			}
		}
		if (!k)
			continue;

		for (int i = 0; i < 3; ++i) {
			HID_set_scancode_state(KA, true);
			HID_commit_state();
			_delay_ms(2);
			_delay_ms(18);
			HID_set_scancode_state(KA, false);
			HID_commit_state();
			_delay_ms(20);
		}
	}

	while (true) {
	}

}

void MAIN_timer_handler()
{
}

void MAIN_handle_sof()
{
}
