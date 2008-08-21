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
		changed_line(y, 0, width-1);
	}
}


void GTerm::shift_text(int y, int start_x, int end_x, int num) {
	if (num == 0)
		return;

	if (num < 0) {
		BufferRow* row = buffer_get_row(&buffer, y);
		bufrow_remove(row,start_x,-num);
	} else {
		// TODO !?:
		//buffer->getRow(y)->insert(start_x, )
	}

	changed_line(y, start_x, end_x);
}

void GTerm::clear_area(int start_x, int start_y, int end_x, int end_y)
{
	const symbol_t style = symbol_make_style(fg_color, bg_color, attributes);
	const symbol_t sym = ' ' | style;

	int w = end_x - start_x + 1;
	if (w<1) {
		return;
	}

	for (int y=start_y; y<=end_y; y++) {
		BufferRow* row = buffer_get_row(&buffer, y);
		bufrow_fill(row, start_x, sym, w);
		changed_line(y, start_x, end_x);
	}
}

void GTerm::changed_line(int y, int start_x, int end_x)
{
	viewport_taint(this, y, start_x, end_x);
}

void GTerm::move_cursor(int x, int y)
{
	if (cursor_x >= width) {
		cursor_x = width-1;
	}
	if (cursor_y >= height) {
		cursor_y = height-1;
	}

	if (x != cursor_x || y != cursor_y) {
		// Old cursor position is dirty
		changed_line(cursor_y, cursor_x, cursor_x+1);

		cursor_x = x;
		cursor_y = y;

		// New cursor position is dirty
		changed_line(cursor_y, cursor_x, cursor_x+1);
	}
}

/* End of File */
