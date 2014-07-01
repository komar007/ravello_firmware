#include "buttons.h"
#include "time.h"

#include <avr/io.h>

static volatile uint8_t state = 0x00;
static volatile uint8_t clicked = 0x00;

void BUTTONS_init()
{
	DDRD &= ~(_BV(PB2) | _BV(PB3) | _BV(PB4) | _BV(PB5) | _BV(PB6));
	PORTD |= _BV(PB2) | _BV(PB3) | _BV(PB4) | _BV(PB5) | _BV(PB6);
}

bool IO_get(uint8_t num)
{
	switch (num) {
	case 0:
		return PIND & _BV(PD5);
	case 1:
		return PIND & _BV(PD6);
	case 2:
		return PIND & _BV(PD4);
	case 3:
		return PIND & _BV(PD3);
	case 4:
		return PIND & _BV(PD2);
	default:
		return false;
	}
}

void BUTTONS_task()
{
	static uint64_t last_time = 0;
	const uint64_t time = TIME_get();
	if (time < last_time + DEBOUNCE_DELAY_MS)
		return;
	last_time = time;
	for (int i = 0; i < 5; ++i) {
		const bool newstate = !IO_get(i);
		if (!(state & (1 << i)) && newstate)
			clicked |= 1 << i;
		if (newstate)
			state |= 1 << i;
		else
			state &= ~(1 << i);
	}
}

bool BUTTONS_get(uint8_t num)
{
	return state & (1 << num);
}

bool BUTTONS_has_clicked(uint8_t num)
{
	const bool c = clicked & (1 << num);
	clicked &= ~(1 << num);
	return c;
}
