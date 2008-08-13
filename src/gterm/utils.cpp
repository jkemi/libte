// Copyright Timothy Miller, 1999

#include "gterm.hpp"
#include "Buffer.h"
#include "Dirty.h"

#include "misc.h"

#include <stdlib.h>

void GTerm::request_redraw(int x, int y, int w, int h, bool force) {
	if (doing_update) {
		printf("bad update!\n");
		return;
	}

	doing_update = true;

	y = int_clamp(y, 0, height-1);
	h = int_clamp(h, 0, height-y);
	x = int_clamp(x, 0, width-1);
	w = int_clamp(w, 0, width-x);


    // then update characters
    for (int rowno = y; rowno < y+h; rowno++) {
		BufferRow* row = buffer->getRow(rowno);

		int dirtstart;
		int dirtend;
		if (force) {
			dirtstart = x;
			dirtend = x+w;
		} else {
			dirtstart = int_max(x, dirty->start[rowno]);
			dirtend = int_min(x+w, dirty->end[rowno]);
		}

		if (dirtend-dirtstart <= 0) {
			continue;
		}
/*
		symbol_t* data = row->data;
		for (int colno = x; colno < x+w;) {
			const symbol_t laststyle = symbol_get_style(data[colno]);
			uint i;
			for (i = colno+1; i < x+w; i++) {
				const symbol_t style = symbol_get_style(data[colno]);
				if (style != laststyle) {
					break;
				}
			}
			const uint runlen = i-colno;

			const symbol_color_t fg = symbol_get_fg(laststyle);
			const symbol_color_t bg = symbol_get_bg(laststyle);
			const symbol_attributes_t attrs = symbol_get_attributes(laststyle);

			DrawText()

			colno += runlen;
		}
*/
		_fe->draw(_fe_priv, dirtstart, rowno, row->data+dirtstart, dirtend-dirtstart);
		dirty->cleanse(rowno, dirtstart, dirtend);
    }

	if (!is_mode_set(CURSORINVISIBLE))
	{
		int xpos = cursor_x;
		if (xpos >= width) {
			xpos = width-1;
		}

		int ypos = cursor_y;

		// draw cursor if force or inside rectangle
		if ( force || (xpos >= x && xpos < x+w && ypos >= y && ypos < y+h) ) {
			const symbol_t sym = buffer->getRow(cursor_y)->data[xpos];

			const symbol_color_t fg = symbol_get_fg(sym);
			const symbol_color_t bg = symbol_get_fg(sym);
			const symbol_attributes_t attrs = symbol_get_attributes(sym);
			const unsigned int cp = symbol_get_codepoint(sym);

			_fe->draw_cursor(_fe_priv, fg, bg, attrs, xpos, ypos, cp);
		}
	}

	doing_update = false;
}

void GTerm::update_changes(void)
{
    // prevent recursion for scrolls which cause exposures
    if (doing_update) {
		return;
    }
    doing_update = true;

    // first perform scroll-copy
    int mx = scroll_bot-scroll_top+1;
    if (!is_mode_set(TEXTONLY) && pending_scroll && (pending_scroll < mx) && (-pending_scroll < mx)) {
		if (pending_scroll < 0) {
			fe_scroll(scroll_top, scroll_bot-scroll_top+pending_scroll+1, (scroll_top - pending_scroll));
		} else {
			fe_scroll(scroll_top + pending_scroll, scroll_bot-scroll_top-pending_scroll+1, scroll_top);
		}
    }
    pending_scroll = 0;


    fe_updated();

    doing_update = false;
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
	const symbol_t style = symbol_make_style(fg_color, bg_color, attributes);
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
	if (cursor_x >= width) {
		cursor_x = width-1;
	}
	if (cursor_y >= height) {
		cursor_y = height-1;
	}
	changed_line(cursor_y, cursor_x, cursor_x);

	cursor_x = x;
	cursor_y = y;
}

/* End of File */
