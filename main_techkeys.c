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

/* whole screen rectangle, for basic text drawing */
const rect_t screen_r = {0, 0, 24, 7};

#define MAX_LEN 50

uint8_t EEMEM ee_strings[4][MAX_LEN+1] = {
	"\x04lhttp://techkeys.us\x0b",
	"\x04lhttp://fb.com/techkeysus\x0b",
	"\x04lmailto:info@techkeys.us\x0b",
	"Hello, World!"
};

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
		} else {
			uint8_t bright;
			int t = TIME_get() % 300;
			if (t < 220)
				bright = min(99, t)/20;
			else
				bright = min(49, (300-t))/10;
			int position;
			if (macro_len <= 4)
				position = 0;
			else
				position = -6*(macro_len - 4);
			GFX_put_text(screen_r, position, 0, macro, macro_len-1, 4, 0);
			GFX_put_text(screen_r, position+6*(macro_len-1), 0, macro+macro_len-1, 1, bright, 0);
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
				--macro[macro_len-1];
				if (macro[macro_len-1] == 'a' - 1)
					macro[macro_len-1] = ' ';
				else if (macro[macro_len-1] == ' ' - 1)
					macro[macro_len-1] = 'z';
				else if (macro[macro_len-1] == 'A' - 1)
					macro[macro_len-1] = 'Z';

				else if (macro[macro_len-1] == '{' - 1)
					macro[macro_len-1] = '`';
				else if (macro[macro_len-1] == '[' - 1)
					macro[macro_len-1] = '@';
				else if (macro[macro_len-1] == ':' - 1)
					macro[macro_len-1] = '/';
				else if (macro[macro_len-1] == '!' - 1)
					macro[macro_len-1] = '9';
				else if (macro[macro_len-1] == '0' - 1)
					macro[macro_len-1] = '~';

				else if (macro[macro_len-1] == -1)
					macro[macro_len-1] = 11;
				break;
			case K_LEFT:
				if (macro_len > 1) {
					macro[macro_len - 1] = 0;
					--macro_len;
				}
				break;
			case K_DOWN:
				++macro[macro_len-1];
				if (macro[macro_len-1] == 'z' + 1)
					macro[macro_len-1] = ' ';
				else if (macro[macro_len-1] == ' ' + 1)
					macro[macro_len-1] = 'a';
				else if (macro[macro_len-1] == 'Z' + 1)
					macro[macro_len-1] = 'A';

				else if (macro[macro_len-1] == '9' + 1)
					macro[macro_len-1] = '!';
				else if (macro[macro_len-1] == '/' + 1)
					macro[macro_len-1] = ':';
				else if (macro[macro_len-1] == '@' + 1)
					macro[macro_len-1] = '[';
				else if (macro[macro_len-1] == '`' + 1)
					macro[macro_len-1] = '{';
				else if (macro[macro_len-1] == '~' + 1)
					macro[macro_len-1] = '0';

				else if (macro[macro_len-1] == 12)
					macro[macro_len-1] = 0;
				break;
			case K_RIGHT:
				//ADD LETTER TO TEMP_STRING
				if (macro_len < MAX_LEN) {
					macro[macro_len] = 'a';
					++macro_len;
					macro[macro_len] = 0;
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
					macro[macro_len-1] = 0;
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
				uint8_t must_release = 0;
				for (int i = 0; i < macro_len; ++i) {
					if (islower(macro[i])) {
						HID_set_scancode_state(KA + macro[i] - 'a', true);
						HID_commit_state();
						TIME_delay_ms(5);
						HID_set_scancode_state(KA + macro[i] - 'a', false);
						HID_commit_state();
						TIME_delay_ms(5);
					} else if (isupper(macro[i])) {
						HID_set_scancode_state(KLEFT_SHIFT, true);
						HID_commit_state();
						TIME_delay_ms(5);
						HID_set_scancode_state(KA + macro[i] - 'A', true);
						HID_commit_state();
						TIME_delay_ms(5);
						HID_set_scancode_state(KA + macro[i] - 'A', false);
						HID_commit_state();
						TIME_delay_ms(5);
						HID_set_scancode_state(KLEFT_SHIFT, false);
						HID_commit_state();
						TIME_delay_ms(5);
					} else if (macro[i] >= 32) {
						bool need_shift = false;
						uint8_t code = 0;
						switch (macro[i]) {
						case '0':
							code = K0;
							break;
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							code = K1 + macro[i] - '1';
							break;
						case ' ':
							code = KSPACE;
							break;
						case '"':
							code = KQUOTE;
							need_shift = true;
							break;
						case '!':
							code = K1;
							need_shift = true;
							break;
						case '@':
							code = K2;
							need_shift = true;
							break;
						case '#':
							code = K3;
							need_shift = true;
							break;
						case '$':
							code = K4;
							need_shift = true;
							break;
						case '%':
							code = K5;
							need_shift = true;
							break;
						case '^':
							code = K6;
							need_shift = true;
							break;
						case '&':
							code = K7;
							need_shift = true;
							break;
						case '*':
							code = K8;
							need_shift = true;
							break;
						case '(':
							code = K9;
							need_shift = true;
							break;
						case ')':
							code = K0;
							need_shift = true;
							break;
						case '_':
							code = KMINUS;
							need_shift = true;
							break;
						case '+':
							code = KEQUAL;
							need_shift = true;
							break;
						case '\'':
							code = KQUOTE;
							break;
						case ',':
							code = KCOMMA;
							break;
						case '-':
							code = KMINUS;
							break;
						case '.':
							code = KPERIOD;
							break;
						case '/':
							code = KSLASH;
							break;
						case ':':
							code = KSEMICOLON;
							need_shift = true;
							break;
						case ';':
							code = KSEMICOLON;
							break;
						case '<':
							code = KCOMMA;
							need_shift = true;
							break;
						case '=':
							code = KEQUAL;
							break;
						case '>':
							code = KPERIOD;
							need_shift = true;
							break;
						case '?':
							code = KSLASH;
							need_shift = true;
							break;
						case '[':
							code = KLEFT_BRACE;
							break;
						case '\\':
							code = KBACKSLASH;
							break;
						case ']':
							code = KRIGHT_BRACE;
							break;
						case '`':
							code = KTILDE;
							break;
						case '{':
							code = KLEFT_BRACE;
							need_shift = true;
							break;
						case '|':
							code = KBACKSLASH;
							need_shift = true;
						case '}':
							code = KRIGHT_BRACE;
							need_shift = true;
							break;
						case '~':
							code = KTILDE;
							need_shift = true;
							break;
						}
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
						uint8_t code = 0;
						bool release = false;
						switch (macro[i]) {
						case 0:
							code = KRIGHT;
							release = true;
							break;
						case 1:
							code = KLEFT;
							release = true;
							break;
						case 2:
							code = KUP;
							release = true;
							break;
						case 3:
							code = KDOWN;
							release = true;
							break;
						case 4:
							must_release = code = KCTRL;
							break;
						case 5:
							must_release = code = KALT;
							break;
						case 6:
							must_release = code = KGUI;
							break;
						case 7:
							break;
						case 8:
							must_release = code = KSHIFT;
							break;
						case 9:
							code = KTAB;
							release = true;
							break;
						case 10:
							code = KESC;
							release = true;
							break;
						case 11:
							code = KENTER;
							release = true;
							break;
						}

						HID_set_scancode_state(code, true);
						HID_commit_state();
						TIME_delay_ms(5);
						if (release) {
							HID_set_scancode_state(code, false);
							HID_commit_state();
							TIME_delay_ms(5);
						}
						continue;
					}
					if (must_release != 0) {
						HID_set_scancode_state(must_release, false);
						HID_commit_state();
						TIME_delay_ms(5);
						must_release = 0;
					}
				}
				if (must_release != 0) {
					HID_set_scancode_state(must_release, false);
					HID_commit_state();
					TIME_delay_ms(5);
				}
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
