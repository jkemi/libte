// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "internal.h"
#include "buffer.h"
#include "viewport.h"
#include "misc.h"

void gt_scroll_region(TE* te, uint start_y, uint end_y, int num)
{
	for (int i = 0; i < num; i++) {
		buffer_scroll_up(&te->buffer, te->scroll_top, te->scroll_bot);
	}
	for (int i = num; i < 0; i++) {
		buffer_scroll_down(&te->buffer, te->scroll_top, te->scroll_bot);
		viewport_history_dec(te);
	}

	if (num > 0) {
		viewport_history_inc(te);
	}
	if (num < 0) {
		viewport_history_dec(te);
	}


//	buffer_scroll(&buffer, start_y, end_y, num);

	for (uint y = start_y; y <= end_y; y++) {
		viewport_taint(te, y, 0, te->width);
	}
}

void gt_clear_area(TE* te, int xpos, int ypos, int width, int height)
{
	const symbol_t style = symbol_make_style(te->fg_color, te->bg_color, te->attributes);
	const symbol_t sym = ' ' | style;

	if (width < 1) {
		return;
	}

	for (int y=ypos; y < ypos+height; y++) {
		BufferRow* row = buffer_get_row(&te->buffer, y);
		bufrow_fill(row, xpos, sym, width);
		viewport_taint(te, y, xpos, width);
	}
}

void gt_move_cursor(TE* te, int x, int y)
{
/*	if (cursor_x >= width) {
		cursor_x = width-1;
	}
	if (cursor_y >= height) {
		cursor_y = height-1;
	}*/
	x = int_clamp(x, 0, te->width-1);
	y = int_clamp(y, 0, te->height-1);

	if (x != te->cursor_x || y != te->cursor_y) {
		// Old cursor position is dirty
		viewport_taint(te, te->cursor_y, te->cursor_x, 1);

		te->cursor_x = x;
		te->cursor_y = y;

		// New cursor position is dirty
		viewport_taint(te, te->cursor_y, te->cursor_x, 1);
	}
}

