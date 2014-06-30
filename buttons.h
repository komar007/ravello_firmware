#pragma once

#include <stdint.h>
#include <stdbool.h>

void BUTTONS_init();
void BUTTONS_set_debounce_delay(uint16_t delay);
void BUTTONS_task();
bool BUTTONS_get(uint8_t num);
