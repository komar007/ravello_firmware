#include "gfx.h"
#include "font.h"
#include "auxiliary.h"

#include <string.h>
#include <avr/pgmspace.h>

#define STRIDE 12

uint8_t buffers[2][7*STRIDE];
uint8_t *work_buf = buffers[0];

void GFX_init()
{
	DISPLAY_init();
	DISPLAY_set_viewport(work_buf, STRIDE);
}

void GFX_swap()
{
	static uint8_t b = 0;
	DISPLAY_set_viewport(work_buf, STRIDE);
	++b;
	work_buf = buffers[b & 1];
	/* clear buffer */
	for (uint8_t i = 0; i < 7*STRIDE; ++i)
		work_buf[i] = 0;
}


void GFX_putpixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t * const buf = work_buf + y*STRIDE + x/2;
	*buf |= color << (4 * (x&1));
}

uint8_t GFX_getpixel(uint8_t x, uint8_t y)
{
	const uint8_t * const pix = work_buf + y*STRIDE + x/2;
	return (*pix >> (4*(x&1))) & 0x0f;
}

void GFX_fill(struct rect bbox, uint8_t color)
{
	for (uint8_t i = bbox.y; i < bbox.y + bbox.h; ++i)
		for (uint8_t j = bbox.x; j < bbox.x + bbox.w; ++j)
			GFX_putpixel(j, i, color);
}

void GFX_put_text(struct rect bbox, int x, int y,
		const char *t, int16_t len, uint8_t fg, uint8_t bg)
{
	int8_t col = 0;
	int8_t firstbit = 0;
	int8_t height = bbox.h;
	int8_t width = bbox.w;
	if (x >= bbox.w || x <= -len*6) {
		return;
	} else if (x > 0) {
		bbox.x += x;
		width -= x;
	} else {
		t += (-x) / 6;
		len -= (-x) / 6;
		col = (-x) % 6;
	}
	if (y >= bbox.h || y <= -7) {
		return;
	} else if (y > 0) {
		bbox.y += y;
		height -= y;
	} else {
		firstbit = -y;
		height += y;
	}

	for (int x = bbox.x, i = 0; i < len && x < bbox.x + width; ++x) {
		if (col < 5) {
			uint8_t letter = pgm_read_byte(&font[5*(*t) + col]);
			letter >>= firstbit;
			for (int y = bbox.y; y < bbox.y + height; ++y, letter >>= 1)
				GFX_putpixel(x, y, (letter & 1) ? fg : bg);
			++col;
		} else {
			for (int y = bbox.y; y < bbox.y + height; ++y)
				GFX_putpixel(x, y, bg);
			col = 0;
			++t; ++i;
		}
	}
}

void GFX_blit_progmem(struct rect bbox, const uint8_t *fbuffer,
		uint8_t stride, uint8_t _x, uint8_t _y)
{
	uint8_t old_x = _x;
	for (uint8_t y = bbox.y; y < bbox.y+bbox.h; ++y, ++_y) {
		_x = old_x;
		for (uint8_t x = bbox.x; x < bbox.x+bbox.w; ++x, ++_x) {
			const uint8_t * const pix = fbuffer + _y*stride + _x/2;
			const uint8_t val = (pgm_read_byte(pix) >> (4*(_x&1))) & 0x0f;
			GFX_putpixel(x, y, val);
		}
	}
}

void GFX_draw_bitmap(struct rect bbox, uint8_t fg, uint8_t bg, const uint8_t *bbuffer,
		uint8_t stride, uint8_t _x, uint8_t _y)
{
	const uint8_t *last_addr = NULL;
	uint8_t last_read = 0;
	for (uint8_t y = bbox.y; y < bbox.y+bbox.h; ++y) {
		for (uint8_t x = bbox.x; x < bbox.x+bbox.w; ++x) {
			const uint8_t rx = _x + x - bbox.x;
			const uint8_t ry = _y + y - bbox.y;
			const uint8_t *addr = bbuffer + ry*stride + rx/8;
			if (addr != last_addr) {
				last_read = pgm_read_byte(addr);
				last_addr = addr;
			}
			GFX_putpixel(x, y, (last_read & (0x80 >> (rx%8))) ? fg : bg);
		}
	}
}
