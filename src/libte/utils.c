// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "gterm.h"
#include "buffer.h"
#include "viewport.h"
#include "misc.h"

void gt_update_changes(GTerm* gt)
{
    gt_fe_updated(gt);
}

void gt_scroll_region(GTerm* gt, uint start_y, uint end_y, int num)
{
	for (int i = 0; i < num; i++) {
		buffer_scroll_up(&gt->buffer, gt->scroll_top, gt->scroll_bot);
	}
	for (int i = num; i < 0; i++) {
		buffer_scroll_down(&gt->buffer, gt->scroll_top, gt->scroll_bot);
		viewport_history_dec(gt);
	}

	if (num > 0) {
		viewport_history_inc(gt);
	}
	if (num < 0) {
		viewport_history_dec(gt);
	}


//	buffer_scroll(&buffer, start_y, end_y, num);

	for (uint y = start_y; y <= end_y; y++) {
		gt_changed_line(gt, y, 0, gt->width);
	}
}

void gt_clear_area(GTerm* gt, int xpos, int ypos, int width, int height)
{
	const symbol_t style = symbol_make_style(gt->fg_color, gt->bg_color, gt->attributes);
	const symbol_t sym = ' ' | style;

	if (width < 1) {
		return;
	}

	for (int y=ypos; y < ypos+height; y++) {
		BufferRow* row = buffer_get_row(&gt->buffer, y);
		bufrow_fill(row, xpos, sym, width);
		gt_changed_line(gt, y, xpos, width);
	}
}

/**
 * Mark portions of line y dirty.
 * \param y	line to taint
 * \param start_x	first dirty col
 * \param len		number to taint
 */
void gt_changed_line(GTerm* gt, int y, int start_x, int len)
{
	viewport_taint(gt, y, start_x, len);
}

void gt_move_cursor(GTerm* gt, int x, int y)
{
/*	if (cursor_x >= width) {
		cursor_x = width-1;
	}
	if (cursor_y >= height) {
		cursor_y = height-1;
	}*/
	x = int_clamp(x, 0, gt->width-1);
	y = int_clamp(y, 0, gt->height-1);

	if (x != gt->cursor_x || y != gt->cursor_y) {
		// Old cursor position is dirty
		gt_changed_line(gt, gt->cursor_y, gt->cursor_x, 1);

		gt->cursor_x = x;
		gt->cursor_y = y;

		// New cursor position is dirty
		gt_changed_line(gt, gt->cursor_y, gt->cursor_x, 1);
	}
}

