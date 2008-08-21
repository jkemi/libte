/*
 * viewport.c
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#include "gterm.hpp"
#include "Dirty.h"
#include "viewport.h"


static void _report_scroll(GTerm* gt) {
	gt->_fe->position(gt->_fe_priv, gt->viewport.offset, history_size(&gt->history));
}


void viewport_init (GTerm* gt, uint w, uint h) {
	gt->viewport.dirty = new Dirty(h, w);
	gt->viewport.updating = false;
	gt->viewport.offset = 1;
	gt->viewport.scroll_lock = false;

	_report_scroll(gt);
}

void viewport_term (GTerm* gt) {
	delete gt->viewport.dirty;
}

void viewport_reshape(GTerm* gt, uint w, uint h) {
	gt->viewport.dirty->reshape(h, w);
}

void viewport_taint (GTerm* gt, uint y, uint start_x, uint end_x) {
	y += gt->viewport.offset;
	if (y >= 0 && y < gt->height) {
		gt->viewport.dirty->setDirty(y, start_x, end_x);
	}
}

void viewport_taint_all	(GTerm* gt) {
	for (uint y = 0; y < gt->height; y++) {
		gt->viewport.dirty->setRowDirt(y);
	}
}

void viewport_move (GTerm* gt, uint y, uint n, int offset) {
	// TODO: implement?
}

void viewport_history_inc(GTerm* gt) {
	if (gt->viewport.offset > 0) {
		if (gt->viewport.scroll_lock) {
			gt->viewport.offset++;
			for (int y = (int)gt->height-(int)gt->viewport.offset; y < gt->height; y++) {
				gt->viewport.dirty->setRowDirt(y);
			}
		} else {
			// TODO: do we need to taint all here?
			gt->viewport.offset = 0;
			viewport_taint_all(gt);
		}
	}

	_report_scroll(gt);
}

void viewport_history_dec(GTerm* gt) {
	uint hsz = history_size(&gt->history);
	if (gt->viewport.offset > 0) {
		if (gt->viewport.scroll_lock) {
			if (gt->viewport.offset > hsz) {
				gt->viewport.offset = hsz;
				viewport_taint_all(gt);
			}
		} else {
			// TODO: do we need to taint all here?
			gt->viewport.offset = 0;
			viewport_taint_all(gt);
		}

	}
	_report_scroll(gt);
}

void viewport_set (GTerm* gt, uint offset) {
	uint hsz = history_size(&gt->history);
	offset = uint_clamp(offset, 0, hsz);

	if (offset != gt->viewport.offset) {
		gt->viewport.offset = offset;
		viewport_taint_all(gt);
		_report_scroll(gt);
	}
}

void viewport_lock_scroll (GTerm* gt, bool lock) {
	gt->viewport.scroll_lock = lock;
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

	symbol_t buf[gt->width];

	int offset = gt->viewport.offset;

    // then update characters
    for (int rowno = y; rowno < y+h; rowno++) {

    	const symbol_t* data;
		int ndata;

    	const int age = offset - rowno;
    	if (age > 0) {
    		data = buf;
    		ndata = history_peek(&gt->history, age-1, buf, x+w);
    	} else {
        	BufferRow* row = buffer_get_row(&gt->buffer, rowno-offset);
    		data = row->data;
    		ndata = row->used;
    	}

		int dirtstart;
		int dirtend;
		if (force) {
			dirtstart = x;
			dirtend = x+w;
		} else {
			dirtstart = int_max(x, dirty->start[rowno]);
			dirtend = int_min(x+w, dirty->end[rowno]);
		}

		const int a = int_max(0, ndata-dirtstart) - int_max(0, ndata-dirtend);
		if (a > 0) {
			gt->_fe->draw_text(gt->_fe_priv, dirtstart, rowno, data+dirtstart, a);
		}
		const int b = int_max(0, dirtend-dirtstart-a);
		if (b > 0) {
			gt->_fe->draw_clear(gt->_fe_priv, dirtstart+a, rowno, SYMBOL_BG_DEFAULT, b);
		}

		dirty->cleanse(rowno, dirtstart, dirtend);
    }

	if (!gt->is_mode_set(MODE_CURSORINVISIBLE)) {

		int xpos = gt->cursor_x;
		int ypos = gt->cursor_y+offset;

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

