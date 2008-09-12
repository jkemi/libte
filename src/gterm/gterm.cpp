// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "Buffer.h"

#include "actions.hpp"

#include "viewport.h"
#include "gterm.hpp"


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

int GTerm::handle_button(te_key_t key)
{
	const char* s = NULL;

	switch (key) {
	case TE_KEY_ENTER:
		if (is_mode_flag(MODE_NEWLINE)) {
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
		if (is_mode_flag(MODE_KEYAPP)) {
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
		fe_send_back(s);
		return 1;
	} else {
		return 0;
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

	if (is_mode_set(MODE_AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer_get_row(&buffer, cursor_y);

			size_t n = uint_min(len, width-cursor_x);
			for (size_t i = 0; i < n; i++) {
				const symbol_t sym = style | text[i];
				syms[i] = sym;
			}

			if (is_mode_set(MODE_INSERT)) {
				bufrow_insert(row, cursor_x, syms, n);
				changed_line(cursor_y, cursor_x, width-cursor_x);
			} else {
				bufrow_replace(row, cursor_x, syms, n);
				changed_line(cursor_y, cursor_x, n);
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
		if (is_mode_set(MODE_INSERT)) {
			bufrow_insert(row, cursor_x, syms, n);
			changed_line(cursor_y, cursor_x, width-cursor_x);
		} else {
			bufrow_replace(row, cursor_x, syms, n);
			changed_line(cursor_y, cursor_x, n);
		}

		move_cursor(cursor_x+n, cursor_y);

		// There were more data than we have remaining space on
		// the line, update last cell
		if (len > n) {
			syms[0] = style | text[len-1];
			bufrow_replace(row, width-1, syms, 1);
		}
	}
}

void GTerm::resize_terminal(int w, int h)
{
	bool* newtabs = new bool[w];
	if (w > width) {
		memset(newtabs+width, 0, sizeof(bool)*(w-width));
	}
	memcpy(newtabs, tab_stops, sizeof(bool)*int_min(width,w));
	tab_stops = newtabs;

/*	clear_area(int_min(width,w), 0, int_max(width,w)-1, h-1);
	clear_area(0, int_min(height,h), w-1, int_min(height,h)-1);*/

	// reset scroll margins
/*	scroll_bot = height-1;
	if (scroll_top >= height) {
		scroll_top = 0;
	}*/

	scroll_top = 0;
	scroll_bot = h-1;

	width = w;
	height = h;

	int cx = int_min(width-1, cursor_x);
	int cy = int_min(height-1, cursor_y);
	move_cursor(cx, cy);

	buffer_reshape(&buffer, h, w);

	viewport_reshape(this, w, h);

	fe_updated();
}

GTerm::GTerm(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	_fe = fe;
	_fe_priv = fe_priv;

	parser_init(this);

	width = w;
	height = h;


	history_init(&history, 1000);
	buffer_init(&buffer, &history, h, w);
	viewport_init(this, w, h);

	// Create tab stops
	tab_stops = new bool[w];
	memset(tab_stops, 0, sizeof(bool)*w);

	cursor_x = 0;
	cursor_y = 0;

	mode_flags = 0;

	// Setup scrolling
	scroll_top = 0;
	scroll_bot = height-1;

	// Setup current attributes
	attributes = 0;
	fg_color = SYMBOL_FG_DEFAULT;
	bg_color = SYMBOL_BG_DEFAULT;

	// Setup flags
	set_mode(MODE_AUTOWRAP);

	clear_area(0, 0, width, height-1);

	stored.attributes = attributes;
	stored.autowrap = true;
	stored.cursor_x = 0;
	stored.cursor_y = 0;
}

GTerm::~GTerm()
{
	buffer_term(&buffer);
	viewport_term(this);
}

void GTerm::fe_send_back(const char* data) {
	// TODO: speedup ?!
	size_t len = strlen(data);
	int32_t buf[len+1];

	for (uint i = 0; i < len+1; i++) {
		buf[i] = data[i];
	}

	_fe->send_back(_fe_priv, buf);
}


void GTerm::fe_request_resize(int width, int height) {
	_fe->request_resize(_fe_priv, width, height);
}

void GTerm::fe_updated(void) {
	_fe->updated(_fe_priv);
}

void GTerm::fe_move(int y, int height, int byoffset) {
	_fe->draw_move(_fe_priv, y, height, byoffset);
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
	return te->gt->width;
}

int te_get_height(TE_Backend* te) {
	return te->gt->height;
}

void te_request_redraw(TE_Backend* te, int x, int y, int w, int h, int force) {
	viewport_request_redraw(te->gt, x, y, w, h, force);
}

void te_process_input(TE_Backend* te, const int32_t* data, size_t len) {
	te->gt->process_input(len, data);
}

int te_handle_button(TE_Backend* te, te_key_t key) {
	return te->gt->handle_button(key);
}

void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers) {
	te->gt->handle_keypress(cp, modifiers);
}

// TODO: remove?
void te_update(TE_Backend* te) {
	te->gt->update_changes();
}

void te_position(TE_Backend* te, int offset) {
	viewport_set(te->gt, offset);
}

void te_lock_scroll(TE_Backend* te, int scroll_lock) {
	viewport_lock_scroll(te->gt, scroll_lock != 0);
}
