#include "buttons.h"
#include "time.h"
#include "io.h"

#include <avr/io.h>

static volatile uint8_t state = 0x00;
static volatile uint8_t clicked = 0x00;

void BUTTONS_init()
{
	for (int i = 0; i < 5; ++i) {
		IO_config(i, INPUT);
		IO_set(i, true);
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
