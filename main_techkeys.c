#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "usb.h"
#include "main.h"
#include "timer.h"
#include "auxiliary.h"
#include "gfx.h"
#include "time.h"
#include "buttons.h"
#include "macro.h"

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

uint8_t EEMEM ee_strings[4][MACRO_MAX_LEN+1] = {
	"Hello, World!",
	"\x05lhttp://techkeys.us\x0c",
	"\x05lhttp://fb.com/techkeysus\x0c",
	"\x05lmailto:info@techkeys.us\x0c"
};

static uint8_t next_symbol(uint8_t symbol)
{
	if (symbol == 'z')
		return ' ';
	else if (symbol == ' ')
		return 'a';
	else if (symbol == 'Z')
		return 'A';

	else if (symbol == '9')
		return '!';
	else if (symbol == '/')
		return ':';
	else if (symbol == '@')
		return '[';
	else if (symbol == '`')
		return '{';
	else if (symbol == '~')
		return '0';

	else if (symbol == 13)
		return 1;

	return symbol+1;
}

static inline uint8_t prev_symbol(uint8_t symbol)
{
	if (symbol == 'a')
		return ' ';
	else if (symbol == ' ')
		return 'z';
	else if (symbol == 'A')
		return 'Z';

	else if (symbol == '{')
		return '`';
	else if (symbol == '[')
		return '@';
	else if (symbol == ':')
		return '/';
	else if (symbol == '!')
		return '9';
	else if (symbol == '0')
		return '~';

	else if (symbol == 1)
		return 13;

	return symbol-1;
}

int main(void)
{
	clock_prescale_set(clock_div_1);

	sei();
	USB_init();

	BUTTONS_init();

	GFX_init();
	uint8_t macro_len = 1;

	/* program edit mode */
	int8_t prog_mode = 0;
	/* after holding "PROGRAM", waiting for key choice to reprogram */
	bool prog_mode_select = false;

	TIME_delay_ms(200);
	for (uint8_t i = 0; i < 40; ++i)
	{
		TIME_delay_ms(30);
		GFX_draw_bitmap(screen_r, 4, 0, robot, 3, 0, i);
		GFX_swap();
	}
	TIME_delay_ms(150);

	int8_t scroll = 0;
	int trans_phase = 0;
	uint64_t transition_start = 0;
	/* the letter the current last letter should morph to or 0 when no
	 * morphing occurs */
	char morphing_to_letter = 0;
	while (true) {
		/* render to screen */
		int scroll_px = 0;
		if (scroll == 1) {
			MACRO_set(macro_len, 'a');
			MACRO_set(macro_len + 1, 0);
			scroll_px = -(int)(TIME_get() - transition_start)*6/150;
			if (scroll_px <= -6) {
				scroll_px = 0;
				++macro_len;
				scroll = 0;
			}
		} else if (scroll == -1) {
			scroll_px = (TIME_get() - transition_start)*6/150;
			if (scroll_px >= 6) {
				scroll_px = 0;
				MACRO_set(macro_len - 1, 0);
				--macro_len;
				scroll = 0;
			}
		} else if (morphing_to_letter) {
			trans_phase = (TIME_get() - transition_start)*5/130;
			if (trans_phase >= 5) {
				trans_phase = 0;
				MACRO_set(macro_len-1, morphing_to_letter);
				morphing_to_letter = 0;
			}
		}

		if (prog_mode_select) {
			GFX_draw_bitmap(screen_r, 4, 0, question, 3, 0, 0);
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
		} else {
			uint8_t brightness;
			int t = TIME_get() % 300;
			if (t < 220)
				brightness = min(99, t)/20;
			else
				brightness = min(49, (300-t))/10;
			int position;
			if (macro_len <= 4)
				position = scroll_px;
			else
				position = scroll_px - 6*(macro_len - 4);
			GFX_put_text(screen_r, position, 0,
					TEXT_EEP, MACRO_get_ptr(), macro_len - 1, 4, 0);
			if (!morphing_to_letter) {
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						TEXT_EEP, MACRO_get_ptr() + macro_len - 1, 1, brightness, 0);
			} else {
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						TEXT_EEP, MACRO_get_ptr() + macro_len - 1, 1, 4 - trans_phase, 0);
				uint8_t tmp[2];
				tmp[0] = morphing_to_letter;
				tmp[1] = 0;
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						TEXT_RAM, tmp, 1, trans_phase, 0);
			}
		}
		GFX_swap();

		/* Poll all the keys */
		int8_t clicked = -1;
		int8_t held = -1;
		int8_t released = -1;
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
				if (!morphing_to_letter && !scroll) {
					morphing_to_letter = prev_symbol(MACRO_get(macro_len-1));
					transition_start = TIME_get();
				}
				break;
			case K_DOWN:
				if (!morphing_to_letter && !scroll) {
					morphing_to_letter = next_symbol(MACRO_get(macro_len-1));
					transition_start = TIME_get();
				}
				break;
			case K_LEFT:
				if (!morphing_to_letter && !scroll && macro_len > 1) {
					if (macro_len <= 4) {
						MACRO_set(--macro_len, 0);
					} else {
						scroll = -1;
						transition_start = TIME_get();
					}
				}
				break;
			case K_RIGHT:
				//ADD LETTER TO TEMP_STRING
				if (!morphing_to_letter && !scroll && macro_len < MACRO_MAX_LEN) {
					if (macro_len < 4) {
						MACRO_set(macro_len, 'a');
						MACRO_set(++macro_len, 0);
					} else {
						scroll = 1;
						transition_start = TIME_get();
					}
				}
				break;
			default:
				break;
			}
			/* check key holds */
			if (held == K_PROG) {
				prog_mode = 0;
			} else if (released == K_PROG) {
				if (islower(MACRO_get(macro_len-1)))
					MACRO_set(macro_len-1, 'A');
				else if (isupper(MACRO_get(macro_len-1)))
					MACRO_set(macro_len-1, '0');
				else if (MACRO_get(macro_len-1) >= 32)
					MACRO_set(macro_len-1, 1);
				else
					MACRO_set(macro_len-1, 'a');
			}
		} else if (prog_mode_select) {
			if (0 <= clicked && clicked <= 3) {
				//UP, DOWN, LEFT, RIGHT ARROW
				prog_mode = clicked+1;
				prog_mode_select = false;
				macro_len = MACRO_init(&ee_strings[clicked][0]);
			} else if (clicked == K_PROG) {
				prog_mode = 0;
				prog_mode_select = false;
			}
		} else { /* regular mode */
			if (held == K_PROG) {
				prog_mode_select = true;
			} else if (clicked == K_PROG) {
				//TODO
			} else if (clicked >= 0) {
				MACRO_init(&ee_strings[clicked][0]);
				MACRO_write();
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
