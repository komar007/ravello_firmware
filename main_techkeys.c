#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "usb_keyboard.h"
#include "hid.h"
#include "timer.h"
#include "auxiliary.h"
#include "gfx.h"
#include "time.h"
#include "buttons.h"

#define K_UP	1
#define K_LEFT	2
#define K_DOWN	3
#define K_RIGHT	4
#define K_PROG	5


volatile bool should_scan = false;

void frame(int row, int frame, int bright)
{
	if (frame == 0)
	{
		GFX_fill((struct rect){12, row, 6, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame ==1) {
		GFX_fill((struct rect){11, row, 6, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 2) {
		GFX_fill((struct rect){10, row, 6, 1}, bright);
		GFX_fill((struct rect){17, row, 3, 1}, bright);
	} else if (frame == 3) {
		GFX_fill((struct rect){9, row, 6, 1}, bright);
		GFX_fill((struct rect){16, row, 2, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 4) {
		GFX_fill((struct rect){8, row, 6, 1}, bright);
		GFX_fill((struct rect){15, row, 2, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 5) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 2, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 6) {
		GFX_putpixel(14, row, bright);
		GFX_fill((struct rect){17, row, 2, 1}, bright);
	} else if (frame == 7) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_putpixel(14, row, bright);
		GFX_fill((struct rect){16, row, 2, 1}, bright);
	} else if (frame == 8) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 3, 1}, bright);
	} else if (frame == 9) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 2, 1}, bright);
	} else if (frame == 10) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_putpixel(14, row, bright);
	} else if (frame == 11) {
		GFX_putpixel(16, row, bright);
	} else if (frame == 12) {
		GFX_putpixel(8, row, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 4, 1}, bright);
	} else if (frame == 13) {
		GFX_fill((struct rect){7, row, 2, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 5, 1}, bright);
	} else if (frame == 14 || frame == 15) {
		GFX_fill((struct rect){6, row, 3, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 6, 1}, bright);
	} else if (frame == 16) {
		GFX_fill((struct rect){5, row, 5, 1}, bright);
		GFX_fill((struct rect){13, row, 6, 1}, bright);
	} else if (frame == 17) {
		GFX_fill((struct rect){5, row, 13, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 18) {
		GFX_fill((struct rect){6, row, 11, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 19) {
		GFX_putpixel(5, row, bright);
		GFX_fill((struct rect){7, row, 9, 1}, bright);
		GFX_fill((struct rect){17, row, 3, 1}, bright);
	} else if (frame == 20) {
		GFX_fill((struct rect){5, row, 2, 1}, bright);
		GFX_fill((struct rect){9, row, 6, 1}, bright);
		GFX_fill((struct rect){16, row, 4, 1}, bright);
	} else if (frame == 21) {
		GFX_fill((struct rect){5, row, 4, 1}, bright);
		GFX_fill((struct rect){11, row, 2, 1}, bright);
		GFX_fill((struct rect){15, row, 5, 1}, bright);
	} else if (frame == 22) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){13, row, 7, 1}, bright);
	} else if (frame == 23 || frame == 24 || frame == 25) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 8, 1}, bright);
	} else if (frame == 26) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 7, 1}, bright);
	} else if (frame == 27) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 4, 1}, bright);
	} else if (frame == 28) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 2, 1}, bright);
	} else if (frame == 29) {
		GFX_fill((struct rect){5, row, 4, 1}, bright);
	} else if (frame == 30) {
		GFX_fill((struct rect){5, row, 2, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	} else if (frame >= 31 && frame <= 35) {
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	} else if (frame == 36) {
		GFX_putpixel(8, row, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_putpixel(14, row, bright);
	} else if (frame == 37) {
		GFX_fill((struct rect){8, row, 2, 1}, bright);
		GFX_fill((struct rect){13, row, 2, 1}, bright);
	} else if (frame == 38) {
		GFX_fill((struct rect){8, row, 7, 1}, bright);
	} else if (frame == 39) {
		GFX_fill((struct rect){9, row, 5, 1}, bright);
	} else if (frame == 40) {
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	}
}

/* whole screen rectangle, for basic text drawing */
const struct rect screen_r = {0, 0, 24, 7};

#define MAX_LEN 10

uint8_t EEMEM ee_strings[4][MAX_LEN+1];

int main(void)
{
	clock_prescale_set(clock_div_1);

	sei();
	USB_init();
	HID_commit_state();

	BUTTONS_init();

	/* initialize eeprom */
	for (int i = 0; i < 4; ++i) {
		const char c = eeprom_read_byte(&ee_strings[i][0]);
		eeprom_busy_wait();
		if (!('a' <= c && c <= 'z')) {
			eeprom_write_byte(&ee_strings[i][0], 'a');
			eeprom_write_byte(&ee_strings[i][1], 0);
			eeprom_busy_wait();
		}
	}

	//TIMER_init();

	GFX_init();
	char macro[MAX_LEN+1] = "a";
	int macro_len = 1;

	/* program edit mode */
	int prog_mode = 0;
	/* after holding "PROGRAM", waiting for key choice to reprogram */
	bool prog_mode_select = false;

	const uint8_t bright = 4;

	TIME_delay_ms(200);
	for (int i =0 ;i<=35 ;++i)
	{
		TIME_delay_ms(20);
		frame(0,i,bright);
		frame(1,i+1,bright);
		frame(2,i+2,bright);
		frame(3,i+3,bright);
		frame(4,i+4,bright);
		frame(5,i+5,bright);
		GFX_swap();
	}
	TIME_delay_ms(150);

	while (true) {
		//Display home text
		if (prog_mode_select) {
			GFX_fill((struct rect){4, 0, 2, 1}, bright);
			GFX_fill((struct rect){18, 0, 2, 1}, bright);
			GFX_putpixel(3, 1, bright);
			GFX_putpixel(6, 1, bright);
			GFX_fill((struct rect){11, 1, 3, 1}, bright);
			GFX_putpixel(17, 1, bright);
			GFX_putpixel(20, 1, bright);
			GFX_putpixel(6, 2, bright);
			GFX_putpixel(11, 2, bright);
			GFX_putpixel(13, 2, bright);
			GFX_putpixel(20, 2, bright);
			GFX_fill((struct rect){4, 3, 2, 1}, bright);
			GFX_fill((struct rect){9, 3, 7, 1}, bright);
			GFX_fill((struct rect){18, 3, 2, 1}, bright);
			GFX_putpixel(4, 4, bright);
			GFX_putpixel(9, 4, bright);
			GFX_putpixel(11, 4, bright);
			GFX_putpixel(13, 4, bright);
			GFX_putpixel(15, 4, bright);
			GFX_putpixel(18, 4, bright);
			GFX_fill((struct rect){9, 5, 7, 1}, bright);
			GFX_putpixel(4, 6, bright);
			GFX_putpixel(18, 6, bright);
			GFX_swap();
		} else if (prog_mode == 0) {
			GFX_put_text(screen_r, 0, 0, "v1.8", 4, bright, 0);
			GFX_swap();
			TIME_delay_ms(5);
		} else {
			if (macro_len <= 4) {
				// Display first 3 entry letters prior to scrolling
				if (TIME_get() % 300 < 220)
					GFX_put_text(screen_r, 0, 0, macro, macro_len, bright, 0);
				else
					GFX_put_text(screen_r, 0, 0, macro, macro_len-1, bright, 0);
			} else {
				// Display text past 3 characters that is scrolled
				int position = (macro_len - 4) * -6;
				if (TIME_get() % 300 < 220)
					GFX_put_text(screen_r, position, 0, macro, macro_len, bright, 0);
				else
					GFX_put_text(screen_r, position, 0, macro, macro_len-1, bright, 0);
			}
			GFX_swap();
		}

		//Poll Keys
		int clicked = 0;
		int held = 0;
		for (int i = 0; i < 5; ++i) {
			if (BUTTONS_has_been_clicked(i))
				clicked = i+1;
			if (BUTTONS_has_been_held(i))
				held = i+1;
		}
		if (!clicked && !held)
			continue;

		if (prog_mode > 0) {
			/* check key clicks */
			switch (clicked) {
			case K_UP:
				//THERE HAS TO BE A BETTER WAY...
				if (--macro[macro_len-1] < 'a')
					macro[macro_len-1] = 'a';
				TIME_delay_ms(150);
				break;
			case K_LEFT:
				//TODO SHORTEN TEMP_STRING BY 1
				if (macro_len > 1) {
					macro[macro_len - 1] = 0;
					--macro_len;
				}
				TIME_delay_ms(300);
				break;
			case K_DOWN:
				if (++macro[macro_len-1] > 'z')
					macro[macro_len-1] = 'z';
				TIME_delay_ms(150);
				break;
			case K_RIGHT:
				//ADD LETTER TO TEMP_STRING
				if (macro_len < MAX_LEN) {
					macro[macro_len] = 'a';
					++macro_len;
					macro[macro_len] = 0;
				}
				TIME_delay_ms(150);
				break;
			default:
				break;
			}
			/* check key holds */
			if (held == K_PROG) {
				eeprom_write_block(macro, &ee_strings[prog_mode-1], MAX_LEN+1);
				eeprom_busy_wait();
				prog_mode = 0;
				TIME_delay_ms(300);
			}
		} else if (prog_mode_select) {
			if (clicked < 5) {
				//UP, DOWN, LEFT, RIGHT ARROW
				prog_mode = clicked;
				prog_mode_select = false;
				/* initialize temp_strig */
				eeprom_read_block(macro, &ee_strings[clicked-1], MAX_LEN+1);
				eeprom_busy_wait();
				macro_len = strlen(macro);

				TIME_delay_ms(300);
			} else {
				//PROG BUTTON
				prog_mode = 0;
				prog_mode_select = false;
				TIME_delay_ms(300);
			}
		} else { /* regular mode */
			if (held == K_PROG) {
				prog_mode_select = true;
				//Delay to avoid immediate escape from prog mode
				TIME_delay_ms(300);
			} else if (clicked == K_PROG) {
				//TODO
			} else if (clicked > 0) {
				eeprom_read_block(macro, &ee_strings[clicked-1], MAX_LEN+1);
				macro_len = strlen(macro);
				eeprom_busy_wait();
				for (int i = 0; i < macro_len; ++i) {
					HID_set_scancode_state(KA + macro[i] - 'a', true);
					HID_commit_state();
					TIME_delay_ms(10);
					HID_set_scancode_state(KA + macro[i] - 'a', false);
					HID_commit_state();
					TIME_delay_ms(10);
				}
				TIME_delay_ms(300);
			}
		}
	}
}


void MAIN_timer_handler()
{
}

void MAIN_handle_sof()
{
	TIME_update_1ms();
	BUTTONS_task();
}
