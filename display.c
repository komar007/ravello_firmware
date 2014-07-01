#include "display.h"

#include <avr/interrupt.h>

static struct viewport viewport;

#define DATA	5
#define SHIFT	2
#define STORE 	4

void DISPLAY_init()
{
	DDRC	= _BV(DATA) | _BV(SHIFT) | _BV(STORE);
	PORTC	= 0x00;
	DDRB    = 0xfe;
	PORTB   = 0x00;

	TCCR1A = 0x00;
	TCCR1B = 0x01; /* no prescaling */
	/* the timer will increment every 1/16 of a milisecond @ 16MHz */
	OCR1A = TCNT1 + 1000;
	TIMSK1 |= _BV(OCIE1A);
	/* load the buffer with zeros */

	PORTC &= ~_BV(DATA);
	for (int i = 0; i < 24; ++i) {
		PORTC |= _BV(SHIFT);
		PORTC &= ~_BV(SHIFT);
	}
}

static const uint16_t slots[] = {10385, 5191, 3542, 3400, 1800, 1500, 600};
ISR(TIMER1_COMPA_vect)
{
	static uint8_t row = 0;
	static uint8_t phase = 0;
	OCR1A = TCNT1 + slots[phase];

	if (phase == 0)
		PORTB &= 0x01;
	PORTC |= _BV(STORE);
	PORTC &= ~_BV(STORE);
	if (phase == 0)
		PORTB |= 0x02 << row;

	/* compute params for the next phase... */
	if (++phase > 6) {
		phase = 0;
		if (++row > 6)
			row = 0;
	}
	/* optimization */
	uint8_t top = 7 - phase;
	uint8_t top2 = top << 4;
	/* ... and fill the buffer with data for the next phase */
	const uint8_t *ptr = &viewport.data[viewport.stride*row];
	for (uint8_t i = 0; i < 12; ++i, ++ptr) {
		const uint8_t val = *ptr;
		/* optimized from: if (7 - (*ptr & 0x0f) <= phase) */
		if ((val & 0x0f) >= top)
			PORTC |= _BV(DATA);
		else
			PORTC &= ~_BV(DATA);
		PORTC |= _BV(SHIFT);
		PORTC &= ~_BV(SHIFT);

		/* optimized from: if (7 - ((*ptr) >> 4) <= phase) */
		if (val >= top2)
			PORTC |= _BV(DATA);
		else
			PORTC &= ~_BV(DATA);
		PORTC |= _BV(SHIFT);
		PORTC &= ~_BV(SHIFT);
	}
}

void DISPLAY_set_viewport(const uint8_t *data, uint8_t stride)
{
	viewport.data = data;
	viewport.stride = stride;
}

