#include "time.h"

static volatile uint64_t time = 0;

void TIME_update_1ms()
{
	++time;
}

uint64_t TIME_get()
{
	return time;
}

void TIME_delay_ms(uint16_t delay)
{
	const uint64_t tstop = TIME_get() + delay;
	while (TIME_get() < tstop)
		;
}
