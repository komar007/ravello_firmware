#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MACRO_MAX_LEN 100

static inline bool is_mod(uint8_t chr)
{
	return chr >= 0x19 && chr <= 0x1d;
}
static inline bool is_sticky_mod(uint8_t chr)
{
	return chr >= 0x05 && chr <= 0x09;
}

int16_t MACRO_init(uint8_t _macro[]);
uint8_t *MACRO_get_ptr();
uint8_t MACRO_get(uint8_t idx);
void MACRO_set(uint8_t idx, uint8_t val);
void MACRO_write(bool should_press, bool should_release);
