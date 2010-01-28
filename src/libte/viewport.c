/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include "misc.h"

#include "internal.h"
#include "viewport_dirty.h"
#include "viewport.h"


struct Viewport_ {
	uint	offset;
	Dirty	dirty;
	bool	updating;
	bool	scroll_lock;
};

static void _report_scroll(TE* te) {
	fe_position(te, te->viewport->offset, history_size(te->history));
}


void viewport_init (TE* te, uint w, uint h) {
	te->viewport = xnew(Viewport, 1);

	dirty_init(&te->viewport->dirty, h, w);
	te->viewport->updating = false;
	te->viewport->offset = 0;
	te->viewport->scroll_lock = false;

	_report_scroll(te);
}

void viewport_term (TE* te) {
	dirty_free(&te->viewport->dirty);
	free(te->viewport);
}

void viewport_reshape(TE* te, uint w, uint h) {
	dirty_reshape(&te->viewport->dirty, h, w);
}

void viewport_taint (TE* te, uint y, uint x, uint len) {
	y += te->viewport->offset;
	if (y >= 0 && y < te->height) {
		dirty_taint(&te->viewport->dirty, y, x, x+len);
	}
}

void viewport_taint_all	(TE* te) {
	for (uint y = 0; y < te->height; y++) {
		dirty_taint_row(&te->viewport->dirty, y);
	}
}

void viewport_report_scroll(TE* te) {
	_report_scroll(te);
}


void viewport_move (TE* te, uint y, uint n, int offset) {
	// TODO: implement?
}

void viewport_history_inc(TE* te) {
	if (te->viewport->offset > 0) {
		if (te->viewport->scroll_lock) {
			te->viewport->offset++;
			for (int y = (int)te->height-(int)te->viewport->offset; y < te->height; y++) {
				dirty_taint_row(&te->viewport->dirty, y);
			}
		} else {
			// TODO: do we need to taint all here?
			te->viewport->offset = 0;
			viewport_taint_all(te);
		}
	}

	_report_scroll(te);
}

void viewport_history_dec(TE* te) {
	uint hsz = history_size(te->history);
	if (te->viewport->offset > 0) {
		if (te->viewport->scroll_lock) {
			if (te->viewport->offset > hsz) {
				te->viewport->offset = hsz;
				viewport_taint_all(te);
			}
		} else {
			// TODO: do we need to taint all here?
			te->viewport->offset = 0;
			viewport_taint_all(te);
		}

	}
	_report_scroll(te);
}

void viewport_set (TE* te, int offset) {
	uint hsz = history_size(te->history);
	const uint off = int_clamp(offset, 0, hsz);

	if (off != te->viewport->offset) {
		te->viewport->offset = off;
		viewport_taint_all(te);
		fe_updated(te);
	}

	_report_scroll(te);
}

void viewport_lock_scroll (TE* te, bool lock) {
	te->viewport->scroll_lock = lock;
}

void viewport_request_redraw(TE* te, int x, int y, int w, int h, bool force) {
	if (te->viewport->updating) {
		DEBUGF("bad update!\n");
		return;
	}
	te->viewport->updating = true;

	y = int_clamp(y, 0, te->height-1);
	h = int_clamp(h, 0, te->height-y);
	x = int_clamp(x, 0, te->width-1);
	w = int_clamp(w, 0, te->width-x);

	symbol_t buf[te->width];

	int offset = te->viewport->offset;

    // then update characters
    for (int rowno = y; rowno < y+h; rowno++) {

    	const symbol_t* data;
		int ndata;

    	const int age = offset - rowno;
    	if (age > 0) {
    		data = buf;
    		ndata = history_peek(te->history, age-1, buf, x+w);
    	} else {
        	BufferRow* row = buffer_get_row(te->buffer, rowno-offset);
    		data = row->data;
    		ndata = row->used;
    	}

		int dirtstart;
		int dirtend;
		if (force) {
			dirtstart = x;
			dirtend = x+w;
		} else {
			dirtstart = int_max(x, te->viewport->dirty.start[rowno]);
			dirtend = int_min(x+w, te->viewport->dirty.end[rowno]);
		}

		const int a = int_max(0, ndata-dirtstart) - int_max(0, ndata-dirtend);
		if (a > 0) {
			fe_draw_text(te, dirtstart, rowno, data+dirtstart, a);
		}
		const int b = int_max(0, dirtend-dirtstart-a);
		if (b > 0) {
			fe_draw_clear(te, dirtstart+a, rowno, SYMBOL_BG_DEFAULT, b);
		}

		dirty_cleanse(&te->viewport->dirty, rowno, dirtstart, dirtend);
    }

	if (!be_is_mode_set(te, MODE_CURSORINVISIBLE)) {

		int xpos = te->cursor_x;
		int ypos = te->cursor_y+offset;

		// draw cursor if force or inside rectangle
		if ( force || (xpos >= x && xpos < x+w && ypos >= y && ypos < y+h) ) {
			// TODO: check row->used, row->capacity here!
			const BufferRow* row = buffer_get_row(te->buffer, te->cursor_y);
			const symbol_t sym = row->data[xpos];

			const symbol_color_t fg = symbol_get_fg(sym);
			const symbol_color_t bg = symbol_get_fg(sym);
			const symbol_attributes_t attrs = symbol_get_attributes(sym);
			const unsigned int cp = symbol_get_codepoint(sym);

			fe_draw_cursor(te, fg, bg, attrs, xpos, ypos, cp);
		}
	}

	te->viewport->updating = false;
}

