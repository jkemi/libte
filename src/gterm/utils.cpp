// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "gterm.hpp"
#include "Buffer.h"
#include "viewport.h"
#include "misc.h"

void GTerm::update_changes(void)
{
    fe_updated();
}

void GTerm::scroll_region(uint start_y, uint end_y, int num)
{
	for (int i = 0; i < num; i++) {
		buffer_scroll_up(&buffer, scroll_top, scroll_bot);
	}
	for (int i = num; i < 0; i++) {
		buffer_scroll_down(&buffer, scroll_top, scroll_bot);
		viewport_history_dec(this);
	}

	if (num > 0) {
		viewport_history_inc(this);
	}
	if (num < 0) {
		viewport_history_dec(this);
	}


//	buffer_scroll(&buffer, start_y, end_y, num);

	for (uint y = start_y; y <= end_y; y++) {
		changed_line(y, 0, width);
	}
}

void GTerm::clear_area(int xpos, int ypos, int width, int height)
{
	const symbol_t style = symbol_make_style(fg_color, bg_color, attributes);
	const symbol_t sym = ' ' | style;

	if (width < 1) {
		return;
	}

	for (int y=ypos; y < ypos+height; y++) {
		BufferRow* row = buffer_get_row(&buffer, y);
		bufrow_fill(row, xpos, sym, width);
		changed_line(y, xpos, width);
	}
}

/**
 * Mark portions of line y dirty.
 * \param y	line to taint
 * \param start_x	first dirty col
 * \param len		number to taint
 */
void GTerm::changed_line(int y, int start_x, int len)
{
	viewport_taint(this, y, start_x, len);
}

void GTerm::move_cursor(int x, int y)
{
/*	if (cursor_x >= width) {
		cursor_x = width-1;
	}
	if (cursor_y >= height) {
		cursor_y = height-1;
	}*/
	x = int_clamp(x, 0, width-1);
	y = int_clamp(y, 0, height-1);

	if (x != cursor_x || y != cursor_y) {
		// Old cursor position is dirty
		changed_line(cursor_y, cursor_x, 1);

		cursor_x = x;
		cursor_y = y;

		// New cursor position is dirty
		changed_line(cursor_y, cursor_x, 1);
	}
}

