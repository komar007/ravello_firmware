#pragma once

#include "display.h"

#include <stdint.h>

void GFX_swap();
void GFX_putpixel(int x, int y, int color);
void GFX_fill(struct rect bbox, uint8_t color);
void GFX_put_text(struct rect bbox, int x, int y,
		const char *t, uint8_t fg, uint8_t bg);
