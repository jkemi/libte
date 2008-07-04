// Copyright Timothy Miller, 1999

#include "gterm.hpp"
#include "Buffer.h"
#include "Dirty.h"
#include <stdlib.h>

static inline int int_min(int a, int b) {return a < b ? a : b;}
static inline int int_max(int a, int b) {return a > b ? a : b;}

int GTerm::calc_color(int fg, int bg, int flags)
{
	fg = fg & 0x7; bg = bg & 0x7;
	return (flags & 0xF) | (fg << 4) | (bg << 8);
}

void GTerm::update_changes()
{
    int blank;

    // prevent recursion for scrolls which cause exposures
    if (doing_update) {
		return;
    }
    doing_update = 1;

    // first perform scroll-copy
    int mx = scroll_bot-scroll_top+1;
    if (!(mode_flags & TEXTONLY) && pending_scroll && (pending_scroll < mx) && (-pending_scroll < mx)) {
		if (pending_scroll < 0) {
		    MoveChars(0, scroll_top, 0, (scroll_top - pending_scroll), width, scroll_bot-scroll_top+pending_scroll+1);
		} else {
		    MoveChars(0, (scroll_top + pending_scroll), 0, scroll_top, width, scroll_bot-scroll_top-pending_scroll+1);
		}
    }
    pending_scroll = 0;

    // then update characters
    for (int y = 0; y < height; y++) {
		BufferRow* row = buffer->getRow(y);

		const int dirtstart = dirty->start[y];
		const int dirtend = dirty->end[y];

		if (dirtend-dirtstart == 0) {
			continue;
		}

		blank = !(mode_flags & TEXTONLY);

		DrawStyledText(dirtstart, y, dirtend-dirtstart, row->data+dirtstart);

		dirty->cleanse(y);
    }

	if (!(mode_flags & CURSORINVISIBLE))
	{
		int x = cursor_x;
		if (x >= width) {
			x = width-1;
		}

		const symbol_t sym = buffer->getRow(cursor_y)->data[x];

		const symbol_color_t fg = symbol_get_fg(sym);
		const symbol_color_t bg = symbol_get_fg(sym);
		const symbol_attributes_t attrs = symbol_get_attributes(sym);
		const unsigned int cp = symbol_get_codepoint(sym);

		DrawCursor(fg, bg, attrs, x, cursor_y, cp);
	}

    doing_update = 0;
}

void GTerm::scroll_region(int start_y, int end_y, int num)
{
/*
	int y, takey, fast_scroll, mx, clr, x, yp, c;
	short temp[GT_MAXHEIGHT];
	unsigned char temp_sx[GT_MAXHEIGHT], temp_ex[GT_MAXHEIGHT];

	if (!num) return;
	mx = end_y-start_y+1;
	if (num > mx) num = mx;
	if (-num > mx) num = -mx;

	fast_scroll = ((start_y == scroll_top) && (end_y == scroll_bot) && !(mode_flags & TEXTONLY));

	if (fast_scroll) pending_scroll += num;

	memcpy(temp, linenumbers, sizeof(linenumbers));
	if (fast_scroll) {
		memcpy(temp_sx, dirty_startx, sizeof(dirty_startx));
		memcpy(temp_ex, dirty_endx, sizeof(dirty_endx));
	}

	c = calc_color(fg_color, bg_color, mode_flags);

	// move the lines by renumbering where they point to
	if ((num < mx) && (-num < mx)) for (y = start_y; y <= end_y; y++)
	{
		takey = y + num;
		clr = (takey < start_y) || (takey > end_y);
		if (takey < start_y) takey = end_y+1-(start_y-takey);
		if (takey > end_y) takey = start_y-1+(takey-end_y);

		linenumbers[y] = temp[takey];
		if (!fast_scroll || clr)
		{
			dirty_startx[y] = 0;
			dirty_endx[y] = width-1;
		}
		else
		{
			dirty_startx[y] = temp_sx[takey];
			dirty_endx[y] = temp_ex[takey];
		}
		if (clr)
		{
			yp = linenumbers[y] * GT_MAXWIDTH;
			memset(text + yp, 32, width);
			for (x = 0; x < width; x++) {
				color[yp++] = c;
			}
		}
	}

	*/
}

void GTerm::shift_text(int y, int start_x, int end_x, int num) {
	if (!num)
		return;

	buffer->getRow(y)->remove(start_x,num);

	changed_line(y, start_x, end_x);
}

void GTerm::clear_area(int start_x, int start_y, int end_x, int end_y)
{
	const symbol_t style = symbol_make_style(fg_color, bg_color, mode_flags);
	const symbol_t sym = ' ' | style;

	int w = end_x - start_x + 1;
	if (w<1) {
		return;
	}

	for (int y=start_y; y<=end_y; y++) {
		buffer->getRow(y)->fill(start_x, sym, w);
		changed_line(y, start_x, end_x);
	}
}

void GTerm::changed_line(int y, int start_x, int end_x)
{
	dirty->setDirty(y, start_x, end_x);
}

void GTerm::move_cursor(int x, int y)
{
	if (cursor_x>=width) cursor_x = width-1;
	if (cursor_y>=height) cursor_y = height-1; // imm
	changed_line(cursor_y, cursor_x, cursor_x);
	cursor_x = x;
	cursor_y = y;
}

void GTerm::set_mode_flag(int flag)
{
	mode_flags |= flag;
	ModeChange(mode_flags);
}

void GTerm::clear_mode_flag(int flag)
{
	mode_flags &= ~flag;
	ModeChange(mode_flags);
}

/* End of File */
