#pragma once

#include <stdint.h>

#define MACRO_MAX_LEN 100

int16_t MACRO_init(uint8_t _macro[]);
uint8_t *MACRO_get_ptr();
uint8_t MACRO_get(uint8_t idx);
void MACRO_set(uint8_t idx, uint8_t val);
void MACRO_write();
