#include "buttons.h"
#include "time.h"
#include "io.h"

#include <avr/io.h>

static volatile int debounce_delay_ms = 0;
static volatile bool state[5] = {false, false, false, false, false};

void BUTTONS_init()
{
	for (int i = 0; i < 5; ++i) {
		IO_config(i, INPUT);
		IO_set(i, true);
	}
}

void BUTTONS_set_debounce_delay(uint16_t delay)
{
	debounce_delay_ms = delay;
}

void BUTTONS_task()
{
	static uint64_t last_time = 0;
	const uint64_t time = TIME_get();
	if (time < last_time + debounce_delay_ms)
		return;
	last_time = time;
	for (int i = 0; i < 5; ++i)
		state[i] = !IO_get(i);
}

bool BUTTONS_get(uint8_t num)
{
	return state[num];
}
