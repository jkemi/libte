/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <stdlib.h>

#include "misc.h"
#include "buffer.h"

#include "actions.h"

#include "viewport.h"
#include "parser.h"
#include "internal.h"


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

	{TE_KEY_HOME,		"\033[H"},	// "\033[1~" for vt220 (or "\033[7~" in rxvt?)
	{TE_KEY_END,		"\033[F"},	// "\033[4~" for vt200 (or "\033[8~" in rxvt?)


	{TE_KP_EQUAL,		"="},
	{TE_KP_DIVIDE,		"/"},
	{TE_KP_MULTIPLY,	"*"},
	{TE_KP_SUBSTRACT,	"-"},
	{TE_KP_ADD,			"+"},
	{TE_KP_PERIOD,		"."},
	{TE_KP_COMMA,		","},
	{TE_KP_ENTER,		"\r"},

	{TE_KP_0,		"0"},
	{TE_KP_1,		"1"},
	{TE_KP_2,		"2"},
	{TE_KP_3,		"3"},
	{TE_KP_4,		"4"},
	{TE_KP_5,		"5"},
	{TE_KP_6,		"6"},
	{TE_KP_7,		"7"},
	{TE_KP_8,		"8"},
	{TE_KP_9,		"9"},

	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_app[] = {
	{TE_KEY_UP,			"\033OA"},
	{TE_KEY_DOWN,		"\033OB"},
	{TE_KEY_RIGHT,		"\033OC"},
	{TE_KEY_LEFT,		"\033OD"},

	{TE_KEY_HOME,		"\033OH"},	// "\033[1~" for vt220 (or "\033[7~" in rxvt?)
	{TE_KEY_END,		"\033OF"},	// "\033[4~" for vt200 (or "\033[8~" in rxvt?)

	{TE_KP_EQUAL,		"\033OX"},
	{TE_KP_DIVIDE,		"\033Oo"},
	{TE_KP_MULTIPLY,	"\033Oj"},	//
	{TE_KP_SUBSTRACT,	"\033Om"},
	{TE_KP_ADD,			"\033Ok"},	//
	{TE_KP_PERIOD,		"\033On"},
	{TE_KP_COMMA,		"\033Ol"},
	{TE_KP_ENTER,		"\033OM"},	//

	{TE_KP_0,		"\033Op"},
	{TE_KP_1,		"\033Oq"},
	{TE_KP_2,		"\033Or"},
	{TE_KP_3,		"\033Os"},
	{TE_KP_4,		"\033Ot"},
	{TE_KP_5,		"\033Ou"},
	{TE_KP_6,		"\033Ov"},
	{TE_KP_7,		"\033Ow"},
	{TE_KP_8,		"\033Ox"},
	{TE_KP_9,		"\033Oy"},

	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_common[] = {
	{TE_KEY_TAB,		"\t"},
	{TE_KEY_ESCAPE,		"\033"},
	{TE_KEY_BACKSPACE,	"\010"},	// ^H

	{TE_KEY_INSERT,		"\033[2~"},
	{TE_KEY_DELETE,		"\033[3~"},
	{TE_KEY_PGUP,		"\033[5~"},
	{TE_KEY_PGDN,		"\033[6~"},

	{(te_key_t)(TE_KEY_F+1),		"\033[11~"},
	{(te_key_t)(TE_KEY_F+2),		"\033[12~"},
	{(te_key_t)(TE_KEY_F+3),		"\033[13~"},
	{(te_key_t)(TE_KEY_F+4),		"\033[14~"},
	{(te_key_t)(TE_KEY_F+5),		"\033[15~"},
	{(te_key_t)(TE_KEY_F+6),		"\033[17~"},
	{(te_key_t)(TE_KEY_F+7),		"\033[18~"},
	{(te_key_t)(TE_KEY_F+8),		"\033[19~"},
	{(te_key_t)(TE_KEY_F+9),		"\033[20~"},
	{(te_key_t)(TE_KEY_F+10),		"\033[21~"},
	{(te_key_t)(TE_KEY_F+11),		"\033[23~"},
	{(te_key_t)(TE_KEY_F+12),		"\033[24~"},
	{(te_key_t)(TE_KEY_F+13),		"\033[25~"},
	{(te_key_t)(TE_KEY_F+14),		"\033[26~"},
	{(te_key_t)(TE_KEY_F+15),		"\033[28~"},
	{(te_key_t)(TE_KEY_F+16),		"\033[29~"},
	{(te_key_t)(TE_KEY_F+17),		"\033[31~"},
	{(te_key_t)(TE_KEY_F+18),		"\033[32~"},
	{(te_key_t)(TE_KEY_F+19),		"\033[33~"},
	{(te_key_t)(TE_KEY_F+20),		"\033[34~"},

	{TE_KEY_UNDEFINED,	NULL}
};

// This receives plain input characters (no line-breaks, control-characters etc)
void be_input(TE* te, const int32_t* text, size_t len) {
	// TODO: remove temporary stack buffer from here..
	symbol_t syms[te->width];
	symbol_t style = symbol_make_style(te->fg_color, te->bg_color, te->attributes);

	if (be_is_mode_set(te, MODE_AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer_get_row(te->buffer, te->cursor_y);

			const size_t n = uint_min(len, te->width-te->cursor_x);
			for (size_t i = 0; i < n; i++) {
				const symbol_t sym = style | text[i];
				syms[i] = sym;
			}

			if (be_is_mode_set(te, MODE_INSERT)) {
				int trail = (te->cursor_x+n)-te->width;
				if (trail > 0) {
					bufrow_remove(row, te->cursor_x+n, trail);
				}

				bufrow_insert(row, te->cursor_x, syms, n);
				viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
			} else {
				bufrow_replace(row, te->cursor_x, syms, n);
				viewport_taint(te, te->cursor_y, te->cursor_x, n);
			}

			be_move_cursor(te, te->cursor_x+n, te->cursor_y);

			len -= n;
			text += n;

			if (len > 0) {
				ac_next_line(te);
			}
		}
	} else {
		BufferRow* row = buffer_get_row(te->buffer, te->cursor_y);

		size_t n = uint_min(len, te->width-te->cursor_x-1);

		for (size_t i = 0; i < n; i++) {
			const symbol_t sym = style | text[i];
			syms[i] = sym;
		}
		if (be_is_mode_set(te, MODE_INSERT)) {
			bufrow_insert(row, te->cursor_x, syms, n);
			viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
		} else {
			bufrow_replace(row, te->cursor_x, syms, n);
			viewport_taint(te, te->cursor_y, te->cursor_x, n);
		}

		be_move_cursor(te, te->cursor_x+n, te->cursor_y);

		// There were more data than we have remaining space on
		// the line, update last cell
		if (len > n) {
			syms[0] = style | text[len-1];
			bufrow_replace(row, te->width-1, syms, 1);
		}
	}
}

void be_scroll_region(TE* te, uint start_y, uint end_y, int num)
{
	for (int i = 0; i < num; i++) {
		buffer_scroll_up(te->buffer, start_y, end_y);
	}
	for (int i = num; i < 0; i++) {
		buffer_scroll_down(te->buffer, start_y, end_y);
	}

	if (num > 0) {
		viewport_history_inc(te);
	}
	if (num < 0) {
		viewport_history_dec(te);
	}

	for (uint y = start_y; y <= end_y; y++) {
		viewport_taint(te, y, 0, te->width);
	}
}

void be_clear_area(TE* te, int xpos, int ypos, int width, int height)
{
	const symbol_t style = symbol_make_style(te->fg_color, te->bg_color, te->attributes);
	const symbol_t sym = ' ' | style;

	if (width < 1) {
		return;
	}

	for (int y=ypos; y < ypos+height; y++) {
		BufferRow* row = buffer_get_row(te->buffer, y);
		bufrow_fill(row, xpos, sym, width);
		viewport_taint(te, y, xpos, width);
	}
}

void be_move_cursor(TE* te, int x, int y)
{
	x = int_clamp(x, 0, te->width-1);
	y = int_clamp(y, 0, te->height-1);

	/*
	if (y > te->scroll_bot || y < te->scroll_top) {
		abort();
	}
	*/

	if (x != te->cursor_x || y != te->cursor_y) {
		// Old cursor position is dirty
		viewport_taint(te, te->cursor_y, te->cursor_x, 1);

		te->cursor_x = x;
		te->cursor_y = y;

		// New cursor position is dirty
		viewport_taint(te, te->cursor_y, te->cursor_x, 1);
	}
}

void be_switch_buffer(TE* te, bool alt) {
	if (alt) {
		te->buffer = &te->alt_buffer;
		te->history = &te->alt_history;
		buffer_clear(&te->alt_buffer);
		viewport_set(te, 0);
	} else {
		te->buffer = &te->norm_buffer;
		te->history = &te->norm_history;
	}

	viewport_taint_all(te);
	viewport_report_scroll(te);
}

TE* te_new(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	TE* te = xnew(TE, 1);

	te->fe = fe;
	te->fe_priv = fe_priv;

	te->parser = parser_new();

	te->width = w;
	te->height = h;


	history_init(&te->norm_history, 1000);
	buffer_init(&te->norm_buffer, &te->norm_history, h, w);

	history_init(&te->alt_history, 0);
	buffer_init(&te->alt_buffer, &te->alt_history, h, w);

	te->buffer = &te->norm_buffer;
	te->history = &te->norm_history;

	viewport_init(te, w, h);

	// Create tab stops
	te->tab_stops = xcnew(bool, w);

	te->cursor_x = 0;
	te->cursor_y = 0;

	te->mode_flags = 0;

	// Setup scrolling
	te->scroll_top = 0;
	te->scroll_bot = te->height-1;

	// Setup current attributes
	te->attributes = 0;
	te->fg_color = SYMBOL_FG_DEFAULT;
	te->bg_color = SYMBOL_BG_DEFAULT;

	// Setup flags
	be_set_mode(te, MODE_AUTOWRAP);

	be_clear_area(te, 0, 0, te->width, te->height-1);

	te->stored.attributes = te->attributes;
	te->stored.autowrap = true;
	te->stored.cursor_x = 0;
	te->stored.cursor_y = 0;

	return te;
}

void fe_send_back_char(TE* te, const char* data) {
	// TODO: speedup ?!
	size_t len = strlen(data);
	int32_t buf[len+1];

	for (uint i = 0; i < len+1; i++) {
		buf[i] = data[i];
	}

	fe_send_back(te, buf);
}

//
// Public API below
//

const char* te_binary_version(void) {
	return TE_SOURCE_VERSION;
}

TE_Backend* te_create(const TE_Frontend* front, void* priv, int width, int height) {
	TE* te = te_new(front, priv, width, height);
	return te;
}

void te_destroy(TE_Backend* te) {
	viewport_term(te);
	buffer_term(&te->norm_buffer);
	history_clear(&te->norm_history);
	buffer_term(&te->alt_buffer);
	history_clear(&te->alt_history);
	parser_delete(te->parser);
}

void te_resize(TE_Backend* te, int width, int height) {
	bool* newtabs = xnew(bool, width);
	if (width > te->width) {
		memset(newtabs+te->width, 0, sizeof(bool)*(width-te->width));
	}
	memcpy(newtabs, te->tab_stops, sizeof(bool)*int_min(te->width,width));
	te->tab_stops = newtabs;

/*	clear_area(int_min(width,w), 0, int_max(width,w)-1, h-1);
	clear_area(0, int_min(height,h), w-1, int_min(height,h)-1);*/

	// reset scroll margins
/*	scroll_bot = height-1;
	if (scroll_top >= height) {
		scroll_top = 0;
	}*/

	te->scroll_top = 0;
	te->scroll_bot = height-1;

	te->width = width;
	te->height = height;

	int cx = int_min(te->width-1, te->cursor_x);
	int cy = int_min(te->height-1, te->cursor_y);
	be_move_cursor(te, cx, cy);

	buffer_reshape(&te->norm_buffer, height, width);
	buffer_reshape(&te->alt_buffer, height, width);

	viewport_reshape(te, width, height);

	fe_updated(te);
}

int te_get_width(TE_Backend* te) {
	return te->width;
}

int te_get_height(TE_Backend* te) {
	return te->height;
}

void te_request_redraw(TE_Backend* te, int x, int y, int w, int h, int force) {
	viewport_request_redraw(te, x, y, w, h, force);
}

void te_process_input(TE_Backend* te, const int32_t* data, size_t len) {
	parser_input(te->parser, len, data, te);
	fe_updated(te);
}

int te_handle_button(TE_Backend* te, te_key_t key) {
	const char* s = NULL;

	switch (key) {
	case TE_KEY_ENTER:
		if (be_is_mode_flag(te, MODE_NEWLINE)) {
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
		if (be_is_mode_flag(te, MODE_KEYAPP)) {
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
		fe_send_back_char(te, s);
		return 1;
	} else {
		return 0;
	}
}

void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers) {
	if (modifiers & TE_MOD_META) {
		int32_t buf[] = {'\033', cp, '\0'};
		fe_send_back(te, buf);
	} else {
		int32_t buf[] = {cp, '\0'};
		fe_send_back(te, buf);
	}
}

void te_position(TE_Backend* te, int offset) {
	viewport_set(te, offset);
}

void te_lock_scroll(TE_Backend* te, int scroll_lock) {
	viewport_lock_scroll(te, scroll_lock != 0);
}

#ifndef NDEBUG
void te_debug(TE_Backend* te, FILE* where) {
	fprintf(where, "dimensions WxH = %dx%d\n", te->width, te->height);
	fprintf(where, "cursor     X,Y = %d,%d\n", te->cursor_x, te->cursor_y);
	fprintf(where, "margins    TOP,BOTTOM = %d,%d\n", te->scroll_top, te->scroll_bot);
	fprintf(where, "buffer: %s\n", (te->buffer == &te->norm_buffer) ? "normal" : "alternative");
	fprintf(where, "buffer     W,H  = %d,%d\n", te->buffer->ncols, te->buffer->nrows);
	fprintf(where, "history           %d (of %d), pos %d\n", te->history->size, te->history->capacity, te->history->pos);
	for (int i = 0; i < te->buffer->nrows; i++) {
		fprintf(where, "line %d : %d (of %d)\n", i, te->buffer->rows[i]->used, te->buffer->rows[i]->capacity);
	}
}
#endif

