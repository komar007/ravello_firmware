#pragma once

#include "display.h"

#include <stdint.h>
#include <avr/pgmspace.h>

void GFX_init();
void GFX_swap();
void GFX_putpixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t GFX_getpixel(uint8_t x, uint8_t y);
void GFX_fill(struct rect bbox, uint8_t color);
void GFX_put_text(struct rect bbox, int x, int y,
		const char *t, int16_t len, uint8_t fg, uint8_t bg);
void GFX_blit_progmem(struct rect bbox, const uint8_t *fbuffer,
		uint8_t stride, uint8_t _x, uint8_t _y);
void GFX_draw_bitmap(struct rect bbox, uint8_t fg, uint8_t bg, const uint8_t *bbuffer,
		uint8_t stride, uint8_t _x, uint8_t _y);
