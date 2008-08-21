/*
 * viewport.c
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#include "gterm.hpp"
#include "Dirty.h"
#include "viewport.h"

void viewport_init (GTerm* gt, uint w, uint h) {
	gt->viewport.dirty = new Dirty(h, w);
	gt->viewport.updating = false;
}

void viewport_term (GTerm* gt) {
	delete gt->viewport.dirty;
}

void viewport_reshape(GTerm* gt, uint w, uint h) {
	gt->viewport.dirty->reshape(h, w);
}

void viewport_taint (GTerm* gt, uint y, uint start_x, uint end_x) {
	y += gt->viewport.offset;
	if ( y >= 0 && y < gt->height) {
		gt->viewport.dirty->setDirty(y, start_x, end_x);
	}
}

void viewport_taint_all	(GTerm* gt) {
	const uint s = int_clamp(0+gt->viewport.offset, 0, gt->height-1);
	const uint e = int_clamp(gt->height+gt->viewport.offset, 0, gt->height-1);

	for (uint y = s; y < e; y++) {
		gt->viewport.dirty->setRowDirt(y);
	}
}

void viewport_move (GTerm* gt, uint y, uint n, int offset) {
	// TODO: implement?
}


void viewport_set (GTerm* gt, uint offset) {
	// TODO: implement?
}

void viewport_request_redraw(GTerm* gt, int x, int y, int w, int h, bool force) {
	if (gt->viewport.updating) {
		printf("bad update!\n");
		return;
	}
	gt->viewport.updating = true;

	y = int_clamp(y, 0, gt->height-1);
	h = int_clamp(h, 0, gt->height-y);
	x = int_clamp(x, 0, gt->width-1);
	w = int_clamp(w, 0, gt->width-x);

	Dirty* dirty = gt->viewport.dirty;

    // then update characters
    for (int rowno = y; rowno < y+h; rowno++) {
    	BufferRow* row = buffer_get_row(&gt->buffer, rowno);

		int dirtstart;
		int dirtend;
		if (force) {
			dirtstart = x;
			dirtend = x+w;
		} else {
			dirtstart = int_max(x, dirty->start[rowno]);
			dirtend = int_min(x+w, dirty->end[rowno]);
		}

		/*
		const int w = dirtend-dirtstart;
		if (w <= 0) {
			continue;
		}
		*/

		const int a = int_max(0, row->used-dirtstart) - int_max(0, row->used-dirtend);
		if (a > 0) {
			gt->_fe->draw(gt->_fe_priv, dirtstart, rowno, row->data+dirtstart, a);
		}
		const int b = int_max(0, dirtend-dirtstart-a);
		if (b > 0) {
			gt->_fe->clear(gt->_fe_priv, dirtstart+a, rowno, SYMBOL_BG_DEFAULT, b);
		}

		dirty->cleanse(rowno, dirtstart, dirtend);
    }

	if (!gt->is_mode_set(MODE_CURSORINVISIBLE))
	{
		int xpos = gt->cursor_x;
		if (xpos >= gt->width) {
			xpos = gt->width-1;
		}

		int ypos = gt->cursor_y;

		// draw cursor if force or inside rectangle
		if ( force || (xpos >= x && xpos < x+w && ypos >= y && ypos < y+h) ) {
			// TODO: check row->used, row->capacity here!
			const BufferRow* row = buffer_get_row(&gt->buffer, gt->cursor_y);
			const symbol_t sym = row->data[xpos];

			const symbol_color_t fg = symbol_get_fg(sym);
			const symbol_color_t bg = symbol_get_fg(sym);
			const symbol_attributes_t attrs = symbol_get_attributes(sym);
			const unsigned int cp = symbol_get_codepoint(sym);

			gt->_fe->draw_cursor(gt->_fe_priv, fg, bg, attrs, xpos, ypos, cp);
		}
	}

	gt->viewport.updating = false;
}

