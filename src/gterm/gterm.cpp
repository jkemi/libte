// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "../strutil.h"

#include "gterm.hpp"

#include "actions.hpp"


extern void parser_init (GTerm* gt);

void GTerm::handle_button(te_key_t key)
{
	switch (key) {
	case TE_KEY_RETURN:
		if (is_mode_flag(GTerm::NEWLINE)) {
			fe_send_back("\r\n");	// send CRLF
		} else {
			fe_send_back("\r");		// ^M (CR)
		}
		break;
	case TE_KEY_HOME:
		fe_send_back("\033[1~");
		break;
	case TE_KEY_INSERT:
		fe_send_back("\033[2~");
		break;
	case TE_KEY_DELETE:
		fe_send_back("\033[3~");
		break;
	case TE_KEY_END:
		fe_send_back("\033[4~");
		break;
	case TE_KEY_PGUP:
		fe_send_back("\033[5~");
		break;
	case TE_KEY_PGDN:
		fe_send_back("\033[6~");
		break;
	};
}

void GTerm::input(const int32_t* text, size_t len) {
	// TODO: remove temporary stack buffer from here..
	symbol_t syms[width];
	symbol_t style = symbol_make_style(fg_color, bg_color, attributes);

	if (is_mode_set(AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer->getRow(cursor_y);

			size_t n = uint_min(len, width-cursor_x);
			for (size_t i = 0; i < n; i++) {
				const symbol_t sym = style | text[i];
				syms[i] = sym;
			}

			if (is_mode_set(INSERT)) {
				row->insert(cursor_x, syms, n);
				changed_line(cursor_y, cursor_x, width);
			} else {
				row->replace(cursor_x, syms, n);
				changed_line(cursor_y, cursor_x, cursor_x+n);
			}

			move_cursor(cursor_x+n, cursor_y);

			len -= n;
			text += n;

			if (len > 0) {
				ac_next_line(this);
			}
		}
	} else {
		BufferRow* row = buffer->getRow(cursor_y);

		size_t n = uint_min(len, width-cursor_x-1);

		for (size_t i = 0; i < n; i++) {
			const symbol_t sym = style | text[i];
			syms[i] = sym;
		}
		if (is_mode_set(INSERT)) {
			row->insert(cursor_x, syms, n);
			changed_line(cursor_y, cursor_x, width);
		} else {
			row->replace(cursor_x, syms, n);
			changed_line(cursor_y, cursor_x, cursor_x+n);
		}

		move_cursor(cursor_x+n, cursor_y);

		// There were more data than we have remaining space on
		// the line, update last cell
		if (len > n) {
			syms[0] = style | text[len-1];
			row->replace(width-1, syms, 1);
		}
	}
}

void GTerm::resize_terminal(int w, int h)
{
	bool* newtabs = new bool[w];
	if (w > width) {
		memset(newtabs+width, 0, sizeof(bool)*(width-w));
	}
	memcpy(newtabs, tab_stops, sizeof(bool)*int_min(width,w));
	tab_stops = newtabs;

	clear_area(int_min(width,w), 0, int_max(width,w)-1, h-1);
	clear_area(0, int_min(height,h), w-1, int_min(height,h)-1);

	width = w;
	height = h;
	scroll_bot = height-1;
	if (scroll_top >= height) {
		scroll_top = 0;
	}
	int cx = int_min(width-1, cursor_x);
	int cy = int_min(height-1, cursor_y);
	move_cursor(cx, cy);

	buffer->reshape(h, w);
	dirty->reshape(h, w);
}

GTerm::GTerm(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	_fe = fe;
	_fe_priv = fe_priv;

	parser_init(this);

	width = w;
	height = h;

	doing_update = false;

	buffer = new Buffer(h, w);
	dirty = new Dirty(h, w);

	// Create tab stops
	tab_stops = new bool[w];
	memset(tab_stops, 0, sizeof(bool)*w);

	cursor_x = 0;
	cursor_y = 0;

	mode_flags = 0;

	// Setup scrolling
	pending_scroll = 0;
	scroll_top = 0;
	scroll_bot = height-1;

	// Setup current attributes
	attributes = 0;
	fg_color = SYMBOL_FG_DEFAULT;
	bg_color = SYMBOL_BG_DEFAULT;

	// Setup flags
	set_mode(GTerm::AUTOWRAP);

	clear_area(0, 0, width, height-1);

	stored.attributes = attributes;
	stored.autowrap = true;
	stored.cursor_x = 0;
	stored.cursor_y = 0;
}

GTerm::~GTerm()
{
	delete buffer;
	delete dirty;
}

void GTerm::fe_send_back(const char* data) {
	// TODO: speedup ?!
	size_t len = str_mbslen(data);
	int32_t buf[len+1];

	size_t nwritten;
	str_mbs_to_cps_n(buf, data, len, strlen(data), &nwritten, NULL);

	buf[nwritten] = L'\0';

	str_mbs_hexdump("sendback mbs: ", data, strlen(data));
	str_cps_hexdump("sendback cps: ", buf, nwritten);

	_fe->send_back(_fe_priv, buf);
}

void GTerm::fe_request_resize(int width, int height) {
	_fe->request_resize(_fe_priv, width, height);
}

void GTerm::fe_updated(void) {
	_fe->updated(_fe_priv);
}

void GTerm::fe_scroll(int y, int height, int offset) {
	_fe->scroll(_fe_priv, y, height, offset);
}

//
// Internal structure
//

struct _TE_Backend {
	GTerm*	gt;
};

//
// Public API below
//

TE_Backend* te_create(const TE_Frontend* front, void* priv, int width, int height) {
	TE_Backend* te = (TE_Backend*)malloc(sizeof(TE_Backend));
	if (te != NULL) {
		te->gt = new GTerm(front, priv, width, height);
	}
	return te;
}

void te_destroy(TE_Backend* te) {
	delete te->gt;
	free(te);
}

void te_resize(TE_Backend* te, int width, int height) {
	te->gt->resize_terminal(width, height);
}

int te_get_width(TE_Backend* te) {
	return te->gt->get_width();
}

int te_get_height(TE_Backend* te) {
	return te->gt->get_height();
}

void te_reqest_redraw(TE_Backend* te, int x, int y, int w, int h, bool force) {
	te->gt->request_redraw(x, y, w, h, force);
}

void te_process_input(TE_Backend* te, const int32_t* data, size_t len) {
#ifndef USE_VTPARSE
	te->gt->process_input(len, data);
#endif
}

void te_process_input_mbs(TE_Backend* te, const char* data, size_t len) {
#ifdef USE_VTPARSE
	vtparse(&te->gt->parser, (const unsigned char*)data, len);
#endif
}

void te_handle_button(TE_Backend* te, te_key_t key) {
	te->gt->handle_button(key);
}

void te_update(TE_Backend* te) {
	te->gt->update_changes();
}
