#include "macro.h"
#include "usb_keyboard.h"
#include "time.h"
#include "hid.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdbool.h>

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

static uint8_t *macro;

/* _macros is a pointer to EE space where macros are stored
 * returns the length of macro, -1 on macro error (not ending with '\0') */
int16_t MACRO_init(uint8_t _macro[])
{
	macro = _macro;
	for (uint16_t i = 0; i < MACRO_MAX_LEN + 1; ++i)
		if (eeprom_read_byte(&macro[i]) == '\0')
			return i;
	return -1;
}

uint8_t *MACRO_get_ptr()
{
	return macro;
}

void MACRO_set(uint8_t idx, uint8_t val)
{
	eeprom_write_byte(&macro[idx], val);
}

uint8_t MACRO_get(uint8_t idx)
{
	return eeprom_read_byte(&macro[idx]);
}

void MACRO_write()
{
	/* keycode scheduled for release after the next keypress or 0 if nothing
	 * should be released */
	uint8_t scheduled_release = 0;
	for (int i = 0; ; ++i) {
		uint8_t byte = eeprom_read_byte(&macro[i]);
		eeprom_busy_wait();
		if (byte == '\0')
			break;
		if (byte >= 32) {
			uint8_t code = pgm_read_byte(&ascii_to_usb_code[byte]);
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
			uint8_t code = pgm_read_byte(&ascii_to_usb_code[byte]);
			bool release = false;
			switch (byte) {
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
			if (byte == 13) {
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
