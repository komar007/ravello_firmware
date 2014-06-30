#pragma once

#include "display.h"

#include <stdint.h>

void GFX_swap();
void GFX_putpixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t GFX_getpixel(uint8_t x, uint8_t y);
void GFX_fill(struct rect bbox, uint8_t color);
void GFX_put_text(struct rect bbox, int x, int y,
		const char *t, int16_t len, uint8_t fg, uint8_t bg);
