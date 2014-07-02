#pragma once

#include <stdint.h>

struct viewport {
	const uint8_t *data;
	uint8_t stride;
};

struct rect {
	int8_t x, y;
	uint8_t w, h;
};

typedef struct rect rect_t;

void DISPLAY_set_viewport(const uint8_t *data, uint8_t stride);
void DISPLAY_init();
