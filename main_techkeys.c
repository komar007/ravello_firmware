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

#define SHIFT_MASK 0x80
const uint8_t PROGMEM ascii_to_usb_code[] = {
	             0,
	             KRIGHT,
	             KLEFT,
	             KUP,
	             KDOWN,
	             KCTRL,
	             KALT,
	             KGUI,
	             0,
	             KSHIFT,
	             KTAB,
	             KESC,
	             KENTER,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             0,
	             KSPACE,
	SHIFT_MASK | K1,
	SHIFT_MASK | KQUOTE,
	SHIFT_MASK | K3,
	SHIFT_MASK | K4,
	SHIFT_MASK | K5,
	SHIFT_MASK | K7,
	             KQUOTE,
	SHIFT_MASK | K9,
	SHIFT_MASK | K0,
	SHIFT_MASK | K8,
	SHIFT_MASK | KEQUAL,
	             KCOMMA,
	             KMINUS,
	             KPERIOD,
	             KSLASH,
	             K0,
	             K1,
	             K2,
	             K3,
	             K4,
	             K5,
	             K6,
	             K7,
	             K8,
	             K9,
	SHIFT_MASK | KSEMICOLON,
	             KSEMICOLON,
	SHIFT_MASK | KCOMMA,
	             KEQUAL,
	SHIFT_MASK | KPERIOD,
	SHIFT_MASK | KSLASH,
	SHIFT_MASK | K2,
	SHIFT_MASK | KA,
	SHIFT_MASK | KB,
	SHIFT_MASK | KC,
	SHIFT_MASK | KD,
	SHIFT_MASK | KE,
	SHIFT_MASK | KF,
	SHIFT_MASK | KG,
	SHIFT_MASK | KH,
	SHIFT_MASK | KI,
	SHIFT_MASK | KJ,
	SHIFT_MASK | KK,
	SHIFT_MASK | KL,
	SHIFT_MASK | KM,
	SHIFT_MASK | KN,
	SHIFT_MASK | KO,
	SHIFT_MASK | KP,
	SHIFT_MASK | KQ,
	SHIFT_MASK | KR,
	SHIFT_MASK | KS,
	SHIFT_MASK | KT,
	SHIFT_MASK | KU,
	SHIFT_MASK | KV,
	SHIFT_MASK | KW,
	SHIFT_MASK | KX,
	SHIFT_MASK | KY,
	SHIFT_MASK | KZ,
	             KLEFT_BRACE,
	             KBACKSLASH,
	             KRIGHT_BRACE,
	SHIFT_MASK | K6,
	SHIFT_MASK | KMINUS,
	             KTILDE,
	             KA,
	             KB,
	             KC,
	             KD,
	             KE,
	             KF,
	             KG,
	             KH,
	             KI,
	             KJ,
	             KK,
	             KL,
	             KM,
	             KN,
	             KO,
	             KP,
	             KQ,
	             KR,
	             KS,
	             KT,
	             KU,
	             KV,
	             KW,
	             KX,
	             KY,
	             KZ,
	SHIFT_MASK | KLEFT_BRACE,
	SHIFT_MASK | KBACKSLASH,
	SHIFT_MASK | KRIGHT_BRACE,
	SHIFT_MASK | KTILDE,
	             0
};

/* whole screen rectangle, for basic text drawing */
const rect_t screen_r = {0, 0, 24, 7};

#define MAX_LEN 50

uint8_t EEMEM ee_strings[4][MAX_LEN+1] = {
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

void macro_write(const char *macro, uint8_t macro_len)
{
	/* keycode scheduled for release after the next keypress or 0 if nothing
	 * should be released */
	uint8_t scheduled_release = 0;
	for (int i = 0; i < macro_len; ++i) {
		if (macro[i] >= 32) {
			uint8_t code = pgm_read_byte(&ascii_to_usb_code[macro[i]]);
			bool need_shift = code & SHIFT_MASK;
			code &= ~SHIFT_MASK;
			if (need_shift) {
				HID_set_scancode_state(KLEFT_SHIFT, true);
				HID_commit_state();
				TIME_delay_ms(5);
			}
			HID_set_scancode_state(code, true);
			HID_commit_state();
			TIME_delay_ms(5);
			HID_set_scancode_state(code, false);
			HID_commit_state();
			TIME_delay_ms(5);
			if (need_shift) {
				HID_set_scancode_state(KLEFT_SHIFT, false);
				HID_commit_state();
				TIME_delay_ms(5);
			}
		} else {
			uint8_t code = pgm_read_byte(&ascii_to_usb_code[macro[i]]);
			bool release = false;
			switch (macro[i]) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 10:
			case 11:
			case 12:
				release = true;
				break;
			case 5:
			case 6:
			case 7:
			case 9:
				scheduled_release = code;
				break;
			case 8:
				break;
			}
			if (macro[i] == 13) {
				TIME_delay_ms(1000);
			} else {
				HID_set_scancode_state(code, true);
				HID_commit_state();
				TIME_delay_ms(5);
				if (release) {
					HID_set_scancode_state(code, false);
					HID_commit_state();
					TIME_delay_ms(5);
				}
			}
			continue;
		}
		if (scheduled_release != 0) {
			HID_set_scancode_state(scheduled_release, false);
			HID_commit_state();
			TIME_delay_ms(5);
			scheduled_release = 0;
		}
	}
	if (scheduled_release != 0) {
		HID_set_scancode_state(scheduled_release, false);
		HID_commit_state();
		TIME_delay_ms(5);
	}
}

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
		if (c == -1) {
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

	TIME_delay_ms(200);
	for (int i = 0; i < 40; ++i)
	{
		TIME_delay_ms(30);
		GFX_draw_bitmap(screen_r, 4, 0, robot, 3, 0, i);
		GFX_swap();
	}
	TIME_delay_ms(150);

	int8_t scroll = 0;
	int scroll_px = 0;
	int trans_phase = 0;
	uint64_t transition_start = 0;
	/* the letter the current last letter should morph to or 0 when no
	 * morphing occurs */
	char morphing_to_letter = 0;
	while (true) {
		if (scroll == 1) {
			macro[macro_len] = 'a';
			macro[macro_len+1] = 0;
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
				macro[macro_len - 1] = 0;
				--macro_len;
				scroll = 0;
			}
		} else if (morphing_to_letter) {
			trans_phase = (TIME_get() - transition_start)*5/130;
			if (trans_phase >= 5) {
				trans_phase = 0;
				macro[macro_len-1] = morphing_to_letter;
				morphing_to_letter = 0;
			}
		}
		//Display home text
		if (prog_mode_select) {
			GFX_draw_bitmap(screen_r, 4, 0,
					question, 3, 0, 0);
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
					macro, macro_len - 1, 4, 0);
			if (!morphing_to_letter) {
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						macro + macro_len - 1, 1, brightness, 0);
			} else {
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						macro + macro_len - 1, 1, 4 - trans_phase, 0);
				char tmp[2];
				tmp[0] = morphing_to_letter;
				tmp[1] = 0;
				GFX_put_text(screen_r, position + 6*(macro_len-1), 0,
						tmp, 1, trans_phase, 0);
			}
		}
		GFX_swap();

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
				if (!morphing_to_letter && !scroll) {
					morphing_to_letter = prev_symbol(macro[macro_len-1]);
					transition_start = TIME_get();
				}
				break;
			case K_DOWN:
				if (!morphing_to_letter && !scroll) {
					morphing_to_letter = next_symbol(macro[macro_len-1]);
					transition_start = TIME_get();
				}
				break;
			case K_LEFT:
				if (!morphing_to_letter && !scroll && macro_len > 1) {
					if (macro_len <= 4) {
						macro[--macro_len] = 0;
					} else {
						scroll = -1;
						transition_start = TIME_get();
					}
				}
				break;
			case K_RIGHT:
				//ADD LETTER TO TEMP_STRING
				if (!morphing_to_letter && !scroll && macro_len < MAX_LEN) {
					if (macro_len < 4) {
						macro[macro_len] = 'a';
						macro[++macro_len] = 0;
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
				eeprom_write_block(macro, &ee_strings[prog_mode-1], MAX_LEN+1);
				eeprom_busy_wait();
				prog_mode = 0;
			} else if (released == K_PROG) {
				if (islower(macro[macro_len-1]))
					macro[macro_len-1] = 'A';
				else if (isupper(macro[macro_len-1]))
					macro[macro_len-1] = '0';
				else if (macro[macro_len-1] >= 32)
					macro[macro_len-1] = 1;
				else
					macro[macro_len-1] = 'a';
			}
		} else if (prog_mode_select) {
			if (0 <= clicked && clicked <= 3) {
				//UP, DOWN, LEFT, RIGHT ARROW
				prog_mode = clicked+1;
				prog_mode_select = false;
				/* initialize temp_strig */
				eeprom_read_block(macro, &ee_strings[clicked], MAX_LEN+1);
				eeprom_busy_wait();
				macro_len = strlen(macro);
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
				eeprom_read_block(macro, &ee_strings[clicked], MAX_LEN+1);
				macro_len = strlen(macro);
				eeprom_busy_wait();
				macro_write(macro, macro_len);
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
