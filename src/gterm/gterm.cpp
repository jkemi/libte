// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"
#include "Dirty.h"

#include "../strutil.h"

#include "gterm.hpp"

#include "actions.hpp"

// key sequence struct, used when translating VT100 key sequences
typedef struct {
	te_key_t	keysym;
	const char*	str;
} keymap;

static const keymap _keys_normal[] = {
	{TE_KEY_UP,			"\033[A"},
	{TE_KEY_DOWN,		"\033[B"},
	{TE_KEY_RIGHT,		"\033[C"},
	{TE_KEY_LEFT,		"\033[D"},
	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_app[] = {
	{TE_KEY_UP,			"\033OA"},
	{TE_KEY_DOWN,		"\033OB"},
	{TE_KEY_RIGHT,		"\033OC"},
	{TE_KEY_LEFT,		"\033OD"},
	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_common[] = {
	{TE_KEY_TAB,		"\t"},
	{TE_KEY_ESCAPE,		"\033"},
	{TE_KEY_BACKSPACE,	"\010"},	// ^H

	{TE_KEY_HOME,		"\033[1~"},	//	"\033[\000" ??
	{TE_KEY_INSERT,		"\033[2~"},
	{TE_KEY_DELETE,		"\033[3~"},
	{TE_KEY_END,		"\033[4~"},	// 	"\033[e" ??
	{TE_KEY_PGUP,		"\033[5~"},
	{TE_KEY_PGDN,		"\033[6~"},

	{te_key_t(TE_KEY_F+1),		"\033[11~"},
	{te_key_t(TE_KEY_F+2),		"\033[12~"},
	{te_key_t(TE_KEY_F+3),		"\033[13~"},
	{te_key_t(TE_KEY_F+4),		"\033[14~"},
	{te_key_t(TE_KEY_F+5),		"\033[15~"},
	{te_key_t(TE_KEY_F+6),		"\033[17~"},
	{te_key_t(TE_KEY_F+7),		"\033[18~"},
	{te_key_t(TE_KEY_F+8),		"\033[19~"},
	{te_key_t(TE_KEY_F+9),		"\033[20~"},
	{te_key_t(TE_KEY_F+10),		"\033[21~"},
	{te_key_t(TE_KEY_F+11),		"\033[23~"},
	{te_key_t(TE_KEY_F+12),		"\033[24~"},
	{te_key_t(TE_KEY_F+13),		"\033[25~"},
	{te_key_t(TE_KEY_F+14),		"\033[26~"},
	{te_key_t(TE_KEY_F+15),		"\033[28~"},
	{te_key_t(TE_KEY_F+16),		"\033[29~"},
	{te_key_t(TE_KEY_F+17),		"\033[31~"},
	{te_key_t(TE_KEY_F+18),		"\033[32~"},
	{te_key_t(TE_KEY_F+19),		"\033[33~"},
	{te_key_t(TE_KEY_F+20),		"\033[34~"},

	{TE_KEY_UNDEFINED,	NULL}
};

extern void parser_init (GTerm* gt);

void GTerm::handle_button(te_key_t key)
{
	const char* s = NULL;

	switch (key) {
	case TE_KEY_RETURN:
		if (is_mode_flag(GTerm::NEWLINE)) {
			s = "\r\n";	//	CRLF
		} else {
			s = "\r";	// ^M (CR)
		}
		break;
	default:
		break;
	}

	if (s == NULL) {
		const keymap* const* tables;
		if (is_mode_flag(KEYAPPMODE)) {
			static const keymap* const t[] = {_keys_app, _keys_common, NULL};
			tables = t;
		} else {
			static const keymap* const t[] = {_keys_normal, _keys_common, NULL};
			tables = t;
		}

		for (const keymap* const* t = tables; s == NULL && t != NULL; t++) {
			for (const keymap* m = *t; s == NULL && m->keysym != TE_KEY_UNDEFINED; m++) {
				if (key == m->keysym) {
					s = m->str;
				}
			}
		}
	}

	if (s != NULL) {
		fe_send_back_simple(s);
	}
}

void GTerm::handle_keypress(int32_t cp, te_modifier_t modifiers) {
	if (modifiers & TE_MOD_META) {
		int32_t buf[] = {'\033', cp, '\0'};
		_fe->send_back(_fe_priv, buf);
	} else {
		int32_t buf[] = {cp, '\0'};
		_fe->send_back(_fe_priv, buf);
	}
}

void GTerm::input(const int32_t* text, size_t len) {
	// TODO: remove temporary stack buffer from here..
	symbol_t syms[width];
	symbol_t style = symbol_make_style(fg_color, bg_color, attributes);

	if (is_mode_set(AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer_get_row(&buffer, cursor_y);

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
		BufferRow* row = buffer_get_row(&buffer, cursor_y);

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

	buffer_reshape(&buffer, h, w);
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

	buffer_init(&buffer, NULL, h, w);
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
	buffer_term(&buffer);
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

void GTerm::fe_send_back_simple(const char* data) {
	// TODO: speedup ?!
	size_t len = strlen(data);
	int32_t buf[len+1];

	for (uint i = 0; i < len+1; i++) {
		buf[i] = data[i];
	}

	str_mbs_hexdump("sendback mbs: ", data, len);

	_fe->send_back(_fe_priv, buf);
}


void GTerm::fe_request_resize(int width, int height) {
	_fe->request_resize(_fe_priv, width, height);
}

void GTerm::fe_updated(void) {
	_fe->updated(_fe_priv);
}

void GTerm::fe_scroll(int y, int height, int byoffset) {
	_fe->move(_fe_priv, y, height, byoffset);
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

void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers) {
	te->gt->handle_keypress(cp, modifiers);
}

void te_update(TE_Backend* te) {
	te->gt->update_changes();
}
