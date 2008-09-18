// Copyright Timothy Miller, 1999

#include <stdlib.h>

#include "misc.h"
#include "buffer.h"

#include "actions.h"

#include "viewport.h"
#include "parser.h"
#include "gterm.h"


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

extern void parser_init (GTerm* gt);

int gt_handle_button(GTerm* gt, te_key_t key)
{
	const char* s = NULL;

	switch (key) {
	case TE_KEY_ENTER:
		if (gt_is_mode_flag(gt, MODE_NEWLINE)) {
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
		if (gt_is_mode_flag(gt, MODE_KEYAPP)) {
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
		gt_fe_send_back(gt, s);
		return 1;
	} else {
		return 0;
	}

}

void gt_handle_keypress(GTerm* gt, int32_t cp, te_modifier_t modifiers) {
	if (modifiers & TE_MOD_META) {
		int32_t buf[] = {'\033', cp, '\0'};
		gt->_fe->send_back(gt->_fe_priv, buf);
	} else {
		int32_t buf[] = {cp, '\0'};
		gt->_fe->send_back(gt->_fe_priv, buf);
	}
}

void gt_input(GTerm* gt, const int32_t* text, size_t len) {
	// TODO: remove temporary stack buffer from here..
	symbol_t syms[gt->width];
	symbol_t style = symbol_make_style(gt->fg_color, gt->bg_color, gt->attributes);

	if (gt_is_mode_set(gt, MODE_AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer_get_row(&gt->buffer, gt->cursor_y);

			size_t n = uint_min(len, gt->width-gt->cursor_x);
			for (size_t i = 0; i < n; i++) {
				const symbol_t sym = style | text[i];
				syms[i] = sym;
			}

			if (gt_is_mode_set(gt, MODE_INSERT)) {
				bufrow_insert(row, gt->cursor_x, syms, n);
				viewport_taint(gt, gt->cursor_y, gt->cursor_x, gt->width-gt->cursor_x);
			} else {
				bufrow_replace(row, gt->cursor_x, syms, n);
				viewport_taint(gt, gt->cursor_y, gt->cursor_x, n);
			}

			gt_move_cursor(gt, gt->cursor_x+n, gt->cursor_y);

			len -= n;
			text += n;

			if (len > 0) {
				ac_next_line(gt);
			}
		}
	} else {
		BufferRow* row = buffer_get_row(&gt->buffer, gt->cursor_y);

		size_t n = uint_min(len, gt->width-gt->cursor_x-1);

		for (size_t i = 0; i < n; i++) {
			const symbol_t sym = style | text[i];
			syms[i] = sym;
		}
		if (gt_is_mode_set(gt, MODE_INSERT)) {
			bufrow_insert(row, gt->cursor_x, syms, n);
			viewport_taint(gt, gt->cursor_y, gt->cursor_x, gt->width-gt->cursor_x);
		} else {
			bufrow_replace(row, gt->cursor_x, syms, n);
			viewport_taint(gt, gt->cursor_y, gt->cursor_x, n);
		}

		gt_move_cursor(gt, gt->cursor_x+n, gt->cursor_y);

		// There were more data than we have remaining space on
		// the line, update last cell
		if (len > n) {
			syms[0] = style | text[len-1];
			bufrow_replace(row, gt->width-1, syms, 1);
		}
	}
}

void gt_resize_terminal(GTerm* gt, int w, int h)
{
	bool* newtabs = xnew(bool, w);
	if (w > gt->width) {
		memset(newtabs+gt->width, 0, sizeof(bool)*(w-gt->width));
	}
	memcpy(newtabs, gt->tab_stops, sizeof(bool)*int_min(gt->width,w));
	gt->tab_stops = newtabs;

/*	clear_area(int_min(width,w), 0, int_max(width,w)-1, h-1);
	clear_area(0, int_min(height,h), w-1, int_min(height,h)-1);*/

	// reset scroll margins
/*	scroll_bot = height-1;
	if (scroll_top >= height) {
		scroll_top = 0;
	}*/

	gt->scroll_top = 0;
	gt->scroll_bot = h-1;

	gt->width = w;
	gt->height = h;

	int cx = int_min(gt->width-1, gt->cursor_x);
	int cy = int_min(gt->height-1, gt->cursor_y);
	gt_move_cursor(gt, cx, cy);

	buffer_reshape(&gt->buffer, h, w);

	viewport_reshape(gt, w, h);

	gt_fe_updated(gt);
}

GTerm* gterm_new(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	GTerm* gt = (GTerm*)malloc(sizeof(GTerm));

	gt->_fe = fe;
	gt->_fe_priv = fe_priv;

	gt->parser = parser_new();

	gt->width = w;
	gt->height = h;


	history_init(&gt->history, 1000);
	buffer_init(&gt->buffer, &gt->history, h, w);
	viewport_init(gt, w, h);

	// Create tab stops
	gt->tab_stops = xnew(bool, w);
	memset(gt->tab_stops, 0, sizeof(bool)*w);

	gt->cursor_x = 0;
	gt->cursor_y = 0;

	gt->mode_flags = 0;

	// Setup scrolling
	gt->scroll_top = 0;
	gt->scroll_bot = gt->height-1;

	// Setup current attributes
	gt->attributes = 0;
	gt->fg_color = SYMBOL_FG_DEFAULT;
	gt->bg_color = SYMBOL_BG_DEFAULT;

	// Setup flags
	gt_set_mode(gt, MODE_AUTOWRAP);

	gt_clear_area(gt, 0, 0, gt->width, gt->height-1);

	gt->stored.attributes = gt->attributes;
	gt->stored.autowrap = true;
	gt->stored.cursor_x = 0;
	gt->stored.cursor_y = 0;

	return gt;
}

void gterm_delete(GTerm* gt) {
	buffer_term(&gt->buffer);
	viewport_term(gt);
	parser_delete(gt->parser);
}

void gt_fe_send_back(GTerm* gt, const char* data) {
	// TODO: speedup ?!
	size_t len = strlen(data);
	int32_t buf[len+1];

	for (uint i = 0; i < len+1; i++) {
		buf[i] = data[i];
	}

	gt->_fe->send_back(gt->_fe_priv, buf);
}


void gt_fe_request_resize(GTerm* gt, int width, int height) {
	gt->_fe->request_resize(gt->_fe_priv, width, height);
	gt_clear_area(gt, 0, 0, width, height);
}

void gt_fe_updated(GTerm* gt) {
	gt->_fe->updated(gt->_fe_priv);
}

void gt_fe_move(GTerm* gt, int y, int height, int byoffset) {
	gt->_fe->draw_move(gt->_fe_priv, y, height, byoffset);
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
		te->gt = gterm_new(front, priv, width, height);
	}
	return te;
}

void te_destroy(TE_Backend* te) {
	gterm_delete(te->gt);
	free(te);
}

void te_resize(TE_Backend* te, int width, int height) {
	gt_resize_terminal(te->gt, width, height);
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
	parser_input(te->gt->parser, len, data, te->gt);
	gt_fe_updated(te->gt);
}

int te_handle_button(TE_Backend* te, te_key_t key) {
	return gt_handle_button(te->gt, key);
}

void te_handle_keypress(TE_Backend* te, int32_t cp, te_modifier_t modifiers) {
	gt_handle_keypress(te->gt, cp, modifiers);
}

void te_position(TE_Backend* te, int offset) {
	viewport_set(te->gt, offset);
}

void te_lock_scroll(TE_Backend* te, int scroll_lock) {
	viewport_lock_scroll(te->gt, scroll_lock != 0);
}
