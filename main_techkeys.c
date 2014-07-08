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

const uint8_t PROGMEM robot[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0F, 0xD0, 0x00, 0x1F, 0xB0,
	0x00, 0x3F, 0x70, 0x00, 0x7E, 0xD0, 0x00, 0xFD,
	0x90, 0x01, 0xFB, 0x30, 0x00, 0x02, 0x60, 0x01,
	0xFA, 0xC0, 0x01, 0xFB, 0x80, 0x01, 0xFB, 0x00,
	0x01, 0xFA, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBB,
	0xC0, 0x01, 0xBB, 0xE0, 0x03, 0xBB, 0xF0, 0x07,
	0xC7, 0xE0, 0x07, 0xFF, 0xD0, 0x03, 0xFF, 0xB0,
	0x05, 0xFF, 0x70, 0x06, 0x7E, 0xF0, 0x07, 0x99,
	0xF0, 0x07, 0xE7, 0xF0, 0x07, 0xEF, 0xF0, 0x07,
	0xEF, 0xE0, 0x07, 0xEF, 0x00, 0x07, 0xEC, 0x00,
	0x07, 0x80, 0x00, 0x06, 0x38, 0x00, 0x00, 0x38,
	0x00, 0x00, 0xBA, 0x00, 0x00, 0xC6, 0x00, 0x00,
	0xFE, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x38, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

const uint8_t PROGMEM techkeys_scroll[] = {
	0xFD, 0xF7, 0xB3, 0xFD, 0xF7, 0xB3, 0x31, 0x86,
	0x3F, 0x31, 0xE6, 0x3F, 0x31, 0x86, 0x33, 0x31,
	0xF7, 0xB3, 0x31, 0xF7, 0xB3, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xEC,
	0xCE, 0xDD, 0xEF, 0xDF, 0xF9, 0x87, 0x98, 0xF1,
	0xC3, 0x0E, 0xD9, 0x83, 0x03, 0xCD, 0xE3, 0x1F,
	0xCD, 0xE3, 0x0E
};

const uint8_t PROGMEM question[] = {
	0x0C, 0x00, 0x30, 0x12, 0x1C, 0x48, 0x02, 0x14,
	0x08, 0x0C, 0x7F, 0x30, 0x08, 0x55, 0x20, 0x00,
	0x7F, 0x00, 0x08, 0x00, 0x20
};	

/* whole screen rectangle, for basic text drawing */
const rect_t screen_r = {0, 0, 24, 7};

#define MAX_LEN 10

uint8_t EEMEM ee_strings[4][MAX_LEN+1];
uint8_t EEMEM ee_modes[4][MAX_LEN+1];

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
			eeprom_write_byte(&ee_modes[i][0], 3);
			eeprom_busy_wait();
		}
	}

	//TIMER_init();

	GFX_init();
	char macro[MAX_LEN+1] = "a";
	uint8_t macro_modes[MAX_LEN+1] = {3, 3, 3, 3, 3, 3};
	int macro_len = 1;

	/* program edit mode */
	int prog_mode = 0;
	/* after holding "PROGRAM", waiting for key choice to reprogram */
	bool prog_mode_select = false;

	const uint8_t bright = 4;

	TIME_delay_ms(200);
	for (int i = 0; i < 40; ++i)
	{
		TIME_delay_ms(30);
		GFX_draw_bitmap(screen_r, 4, 0, robot, 3, 0, i);
		GFX_swap();
	}
	TIME_delay_ms(150);

	while (true) {
		//Display home text
		if (prog_mode_select) {
			GFX_draw_bitmap(screen_r, 4, 0,
					question, 3, 0, 0);
			GFX_swap();
		} else if (prog_mode == 0) {
			const int t = TIME_get() % 7000;
			if (t < 3000)
				GFX_draw_bitmap(screen_r, 2, 0,
						techkeys_scroll, 3, 0, 0);
			else if (t < 3500)
				GFX_draw_bitmap(screen_r, 2, 0,
						techkeys_scroll, 3, 0, (t-3000) / 50);
			else if (t < 6500)
				GFX_draw_bitmap(screen_r, 2, 0,
						techkeys_scroll, 3, 0, 10);
			else
				GFX_draw_bitmap(screen_r, 2, 0,
						techkeys_scroll, 3, 0, (7000-t) / 50);
			GFX_swap();
			TIME_delay_ms(30);
		} else {
			if (macro_len <= 4) {
				// Display first 3 entry letters prior to scrolling
				if (TIME_get() % 300 < 220)
					GFX_put_text(screen_r, 0, 0, macro, macro_len, bright, 0);
				else
					GFX_put_text(screen_r, 0, 0, macro, macro_len-1, bright, 0);
				for (uint8_t i = 0; i < macro_len; ++i) {
					if (macro_modes[i] & 0x01) {
						GFX_putpixel(i*6+5, 6, 2);
						GFX_putpixel(i*6+5, 5, 1);
					}
					if (macro_modes[i] & 0x02) {
						GFX_putpixel(i*6+5, 0, 2);
						GFX_putpixel(i*6+5, 1, 1);
					}
				}
			} else {
				// Display text past 3 characters that is scrolled
				int position = (macro_len - 4) * -6;
				if (TIME_get() % 300 < 220)
					GFX_put_text(screen_r, position, 0, macro, macro_len, bright, 0);
				else
					GFX_put_text(screen_r, position, 0, macro, macro_len-1, bright, 0);
				for (uint8_t i = 0; i < 4; ++i) {
					if (macro_modes[macro_len - 4 + i] & 0x01) {
						GFX_putpixel(i*6+5, 6, 2);
						GFX_putpixel(i*6+5, 5, 1);
					}
					if (macro_modes[macro_len - 4 + i] & 0x02) {
						GFX_putpixel(i*6+5, 0, 2);
						GFX_putpixel(i*6+5, 1, 1);
					}
				}
			}
			GFX_swap();
		}

		//Poll Keys
		int clicked = -1;
		int held = -1;
		int released = -1;
		for (int i = 0; i < 5; ++i) {
			if (BUTTONS_has_been_clicked(i))
				clicked = i;
			if (BUTTONS_has_been_held(i))
				held = i;
			if (BUTTONS_has_been_released(i))
				released = i;
		}
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
					macro_modes[macro_len] = 3;
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
				eeprom_write_block(macro_modes, &ee_modes[prog_mode-1], MAX_LEN+1);
				eeprom_busy_wait();
				prog_mode = 0;
				TIME_delay_ms(300);
			} else if (released == K_PROG) {
				if (++macro_modes[macro_len - 1] > 3)
					macro_modes[macro_len - 1] = 1;
			}
		} else if (prog_mode_select) {
			if (0 <= clicked && clicked <= 3) {
				//UP, DOWN, LEFT, RIGHT ARROW
				prog_mode = clicked+1;
				prog_mode_select = false;
				/* initialize temp_strig */
				eeprom_read_block(macro, &ee_strings[clicked], MAX_LEN+1);
				eeprom_read_block(macro_modes, &ee_modes[clicked], MAX_LEN+1);
				eeprom_busy_wait();
				macro_len = strlen(macro);
				TIME_delay_ms(300);
			} else if (clicked == K_PROG) {
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
			} else if (clicked >= 0) {
				eeprom_read_block(macro, &ee_strings[clicked], MAX_LEN+1);
				eeprom_read_block(macro_modes, &ee_modes[clicked], MAX_LEN+1);
				macro_len = strlen(macro);
				eeprom_busy_wait();
				for (int i = 0; i < macro_len; ++i) {
					if (macro_modes[i] & 0x01) {
						HID_set_scancode_state(KA + macro[i] - 'a', true);
						HID_commit_state();
						TIME_delay_ms(10);
					}
					if (macro_modes[i] & 0x02) {
						HID_set_scancode_state(KA + macro[i] - 'a', false);
						HID_commit_state();
						TIME_delay_ms(10);
					}
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
