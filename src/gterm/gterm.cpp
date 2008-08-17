// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "../strutil.h"

#include "gterm.hpp"


extern void parser_init (GTerm* gt);

void GTerm::handle_button(te_key_t key)
{
switch (key) {
case TE_KEY_RETURN: {
	if (is_mode_flag(GTerm::NEWLINE)) {
		fe_send_back("\r\n");	// send CRLF if GTerm::NEWLINE is set
	} else {
		fe_send_back("\r");	// ^M (CR)
	}
}
};
}

size_t GTerm::input(const int32_t* text, size_t len) {
	// TODO: this check doesn't work and should probably not be here to begin with...
	if (cursor_x >= width) {
		if (is_mode_set(NOEOLWRAP)) {
			cursor_x = width-1;
		} else {
			if (cursor_y < scroll_bot) {
				move_cursor(0, cursor_y+1);
			} else {
				scroll_region(scroll_top, scroll_bot, 1);
			}
		}
	}

	// Count number of consumed bytes and number of bytes to print
	size_t n;		// number of bytes to draw
	size_t n_taken;	// number of bytes consumed from input data stream

	if (is_mode_set(NOEOLWRAP)) {
		n = uint_min(width-cursor_x, len);
		n_taken = len;
	} else {
		n = uint_min(width-cursor_x, len);
		n_taken = n;
	}

	// TODO: remove temporary stack buffer from here..
	symbol_t syms[n];
	symbol_t style = symbol_make_style(fg_color, bg_color, attributes);
	for (size_t i = 0; i < n; i++) {
		const symbol_t sym = style | text[i];
		syms[i] = sym;
	}
	BufferRow* row = buffer->getRow(cursor_y);

	if (is_mode_set(INSERT)) {
		row->insert(cursor_x, syms, n);
		changed_line(cursor_y, cursor_x, width);
	} else {
		row->replace(cursor_x, syms, n);
		changed_line(cursor_y, cursor_x, cursor_x+n);
	}

	cursor_x += n;

	return n_taken;
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
	save_x = 0;
	save_y = 0;
	mode_flags = 0;

	// Setup scrolling
	pending_scroll = 0;
	scroll_top = 0;
	scroll_bot = height-1;

	// Setup current attributes
	attributes = 0;
	fg_color = 7;
	bg_color = 0;

	// Setup flags
	set_mode(GTerm::NOEOLWRAP);

	clear_area(0, 0, width, height-1);
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

	str_mbs_hexdump("mbs: ", data, strlen(data));
	str_cps_hexdump("cps: ", buf, nwritten);

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
