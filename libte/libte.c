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

static const keymap _keys_cursor_keys_on[] = {
	{TE_KEY_UP,			"\033OA"},
	{TE_KEY_DOWN,		"\033OB"},
	{TE_KEY_RIGHT,		"\033OC"},
	{TE_KEY_LEFT,		"\033OD"},

	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_cursor_keys_off[] = {
	{TE_KEY_UP,			"\033[A"},
	{TE_KEY_DOWN,		"\033[B"},
	{TE_KEY_RIGHT,		"\033[C"},
	{TE_KEY_LEFT,		"\033[D"},

	{TE_KEY_UNDEFINED,	NULL}
};

static const keymap _keys_normal[] = {
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

static inline int32_t _cp_replace(const te_chartable_entry_t* charset, int32_t cp) {
	// Find replacement in selected charset
	for (const te_chartable_entry_t* ce = charset; ce->from != '\0'; ce++) {
		if (ce->from == cp) {
			return ce->to;
		}
	}

	return cp;
}

// This receives plain input characters (no line-breaks, control-characters etc)
void be_input(TE* te, const int32_t* text, size_t len) {
	// TODO: remove temporary stack buffer from here..
	symbol_t syms[te->width];
	symbol_t style = symbol_make_style(te->fg_color, te->bg_color, te->attributes);

	const te_chartable_entry_t* charset = (te->charset == 0) ? te->charset_g0 : te->charset_g1;
	
	if (be_is_mode_set(te, MODE_AUTOWRAP)) {
		while (len > 0) {
			BufferRow* row = buffer_get_row(te->buffer, te->cursor_y);

			const size_t n = uint_min(len, te->width-te->cursor_x);
			for (size_t i = 0; i < n; i++) {
				// Find replacement in selected charset
				const int32_t cp = _cp_replace(charset, text[i]);
				const symbol_t sym = style | cp;
				syms[i] = sym;
			}

			if (be_is_mode_set(te, MODE_INSERT)) {
				int trail = (te->cursor_x+n)-te->width;
				if (trail > 0) {
					bufrow_remove(row, te->cursor_x+n, trail, te->width);
				}

				bufrow_insert(row, te->cursor_x, syms, n, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
				viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
			} else {
				bufrow_replace(row, te->cursor_x, syms, n, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
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
			const int32_t cp = _cp_replace(charset, text[i]);
			const symbol_t sym = style | cp;
			syms[i] = sym;
		}
		if (be_is_mode_set(te, MODE_INSERT)) {
			bufrow_insert(row, te->cursor_x, syms, n, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
			viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
		} else {
			bufrow_replace(row, te->cursor_x, syms, n, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
			viewport_taint(te, te->cursor_y, te->cursor_x, n);
		}

		be_move_cursor(te, te->cursor_x+n, te->cursor_y);

		// There were more data than we have remaining space on
		// the line, update last cell
		if (len > n) {
			syms[0] = style | _cp_replace(charset, text[len-1]);
			bufrow_replace(row, te->width-1, syms, 1, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
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
		bufrow_fill(row, xpos, sym, width, symbol_make_style(te->fg_color, te->bg_color, te->attributes));
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

void be_switch_buffer(TE* te, bool alt, bool erase_display_on_alt) {
	if (alt) {
		te->buffer = &te->alt_buffer;
		te->history = &te->alt_history;
		if (erase_display_on_alt) {
			buffer_clear(&te->alt_buffer);
		}
		viewport_set(te, 0);
	} else {
		te->buffer = &te->norm_buffer;
		te->history = &te->norm_history;
	}

	viewport_taint_all(te);
	viewport_report_scroll(te);
}

void be_screen_mode(TE* te, bool enable) {
	if (be_is_mode_flag(te, MODE_SCREEN) == enable) {
		return;
	}
	
	if (enable) {
		be_set_mode_flag(te, MODE_SCREEN);
	} else {
		be_clear_mode_flag(te, MODE_SCREEN);
	}

	te_color_t tmp = te->palette[TE_COLOR_TEXT_FG];
	te->palette[TE_COLOR_TEXT_FG] = te->palette[TE_COLOR_TEXT_BG];
	te->palette[TE_COLOR_TEXT_BG] = tmp;
	fe_palette(te, TE_COLOR_TEXT_FG, 2, te->palette+TE_COLOR_TEXT_FG);
	viewport_taint_colors(te, TE_COLOR_TEXT_FG, 2);
}

TE* te_new(const TE_Frontend* fe, void* fe_priv, int w, int h)
{
	TE* te = xnew(TE, 1);

	te->fe = fe;
	te->fe_priv = fe_priv;

	te->parser = parser_new();

	te->width = w;
	te->height = h;


	history_init(&te->norm_history, 1000);		// TODO: make configurable
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
	te->fg_color = TE_COLOR_TEXT_FG;
	te->bg_color = TE_COLOR_TEXT_BG;

	// Setup flags
	be_set_mode(te, MODE_AUTOWRAP);

	be_clear_area(te, 0, 0, te->width, te->height-1);

	te->charset = 0;
	te->charset_g0 = chartable_us;
	te->charset_g1 = chartable_us;

	te->stored.attributes = te->attributes;
	te->stored.fg = te->fg_color;
	te->stored.bg = te->bg_color;
	te->stored.autowrap = true;
	te->stored.cursor_x = 0;
	te->stored.cursor_y = 0;
	te->stored.charset = 0;
	te->stored.charset_g0 = chartable_us;
	te->stored.charset_g1 = chartable_us;

	te->mouse_mode = MOUSE_TRACK_NONE;
	te->mouse_x = -1;
	te->mouse_y = -1;
	te->mouse_buttons = TE_MOUSE_NONE;

	te->selstate = SELSTATE_NONE;

	const te_color_t defpal[] = {
		{229,229,229},	// TEXT_FG
		{64,64,64},	// TEXT_BG
		{},	// MOUSE_FG
		{},	// MOUSE_BG
		{},	// SEL_FG
		{}, // SEL_BG

		// ANSI (PC colors from: http://en.wikipedia.org/wiki/ANSI_escape_code)
		{0,0,0},		// BLACK
		{170,0,0},		// RED
		{0,170,0},		// GREEN
		{170,170,0},	// YELLOW
		{0,0,170},		// BLUE
		{170,0,170},	// MAGENTA
		{0,170,170},	// CYAN
		{170,170,170},	// GRAY

#if (TE_COLOR_MODE > 8)
		{85,85,85},		// DARKGRAY
		{255,85,85},	// RED
		{85,255,85},	// GREEN
		{255,255,85},	// YELLOW
		{85,85,255},	// BLUE
		{255,85,255},	// MAGENTA
		{85,255,255},	// CYAN
		{255,255,255},	// WHITE
#endif
	};

	memcpy(te->palette, defpal, sizeof(defpal));
#if (TE_COLOR_MODE > 88)
	// create 256 color map
	// colors 16-231 are a 6x6x6 color cube
	te_color_t pal256[256-16];
	const uint8_t ramp[] = {0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
	for (uint red = 0; red < 6; red++) {
		for (uint green = 0; green < 6; green++) {
			for (uint blue = 0; blue < 6; blue++) {
				uint index = red*36+green*6+blue;
//				pal256[index].r = (red * 85)/2;	// fixed point mult by 42.5
//				pal256[index].g = (green * 85)/2;
//				pal256[index].b = (blue * 85)/2;
				pal256[index].r = ramp[red];
				pal256[index].g = ramp[green];
				pal256[index].b = ramp[blue];
			}
		}
	}
	
	// colors 232-255 are a grayscale ramp, intentionally leaving out black and white
	for (uint gray = 0; gray < 24; gray++) {
		uint level = (gray * 10) + 8;
		uint index = 232-16+gray;
		pal256[index].r = level;
		pal256[index].g = level;
		pal256[index].b = level;
	}
	
	memcpy(te->palette+(TE_COLOR_ANSI+16), pal256, sizeof(pal256));
#elif (TE_COLOR_MODE > 16)
	// create 88 color map
	// colors 16-79 are a 4x4x4 color cube
	te_color_t pal88[88-16];
	const uint8_t ramp[] = {0, 0x8b, 0xcd, 0xff};
	for (uint red = 0; red < 4; red++) {
		for (uint green = 0; green < 4; green++) {
			for (uint blue = 0; blue < 4; blue++) {
				uint index = red*16+green*4+blue;
				pal88[index].r = ramp[red];
				pal88[index].g = ramp[green];
				pal88[index].b = ramp[blue];
			}
		}
	}
	
	// colors 80-87 are a grayscale ramp, intentionally leaving out black and white
	for (uint gray = 0; gray < 8; gray++) {
		uint level = ((gray+1)*256)/10;
		uint index = 80-16+gray;
		pal88[index].r = level;
		pal88[index].g = level;
		pal88[index].b = level;
	}
	
	memcpy(te->palette+(TE_COLOR_ANSI+16), pal88, sizeof(pal88));	
#endif
	
	return te;
}

void fe_send_back_char(TE* te, const char* data) {
	// TODO: speedup ?!
	size_t len = strlen(data);
	int32_t buf[len+1];

	for (uint i = 0; i < len+1; i++) {
		buf[i] = data[i];
	}

	fe_send_back(te, buf, len);
}

//
// Public API below
//

const char*	te_binary_version_string = TE_HEADER_VERSION;
const int	te_binary_version[3] = {TE_HEADER_VERSION_MAJOR, TE_HEADER_VERSION_MINOR, TE_HEADER_VERSION_FIX};


TE_Backend* te_create(const TE_Frontend* front, void* user, int width, int height, const void* options, size_t options_size) {
	TE* te = te_new(front, user, width, height);
	return te;
}

void te_destroy(TE_Backend* te) {
	viewport_term(te);
	buffer_term(&te->norm_buffer);
	history_term(&te->norm_history);
	buffer_term(&te->alt_buffer);
	history_term(&te->alt_history);
	parser_delete(te->parser);
	free(te->tab_stops);
	free(te);
}

void te_alter_palette(TE_Backend* te, int first, int count, const te_color_t* data) {
	count = uint_min(count, TE_COLOR_COUNT);
	memcpy(te->palette+first,data,sizeof(te_color_t)*count);
	viewport_taint_colors(te, first, count);
	fe_updated(te);
}

const te_color_t* te_get_palette(TE_Backend* te) {
	return te->palette;
}

void te_resize(TE_Backend* te, int width, int height) {
	bool* newtabs = xnew(bool, width);
	if (width > te->width) {
		memset(newtabs+te->width, 0, sizeof(bool)*(width-te->width));
	}
	memcpy(newtabs, te->tab_stops, sizeof(bool)*int_min(te->width,width));
	free(te->tab_stops);
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
#if 0
	fprintf(stderr, "input %ld cps\n", len);
	for (uint i=0; i<len; i++) {
		int32_t cp = data[i];
		if (cp >= 32 && cp<128) {
			fprintf(stderr, " %d: %d ('%c')\n", i, cp, cp);
		} else {
			fprintf(stderr, " %d: %d\n", i, cp);
		}
	}
#endif
	parser_input(te->parser, len, data, te);
	fe_updated(te);
}

int te_handle_button(TE_Backend* te, te_key_t key, te_modifier_t modifiers) {
	// TODO: handle modifier + F_* keys
	const char* s = NULL;

	switch (key) {
	case TE_KEY_ENTER:
		if (be_is_mode_flag(te, MODE_NEWLINE)) {
			s = "\r\n";	// CRLF
		} else {
			s = "\r";	// ^M (CR)
		}
		break;
	case TE_KEY_BACKSPACE:
		// in order to be compat with xterm etc, make backspace into delete
		// backspace->delete, shift+backspace->backspace
		if ((modifiers & TE_MOD_SHIFT) == 0) {
			s = "\177";	// DEL
		}
		break;
	default:
		break;
	}

	if (s == NULL) {
		const keymap* const tables[] = {
				be_is_mode_flag(te, MODE_CURSORAPP) ? _keys_cursor_keys_on : _keys_cursor_keys_off,
				be_is_mode_flag(te, MODE_KEYAPP) ? _keys_app : _keys_normal,
				_keys_common,
				NULL
		};

		for (const keymap* const* t = tables; s == NULL && t != NULL; t++) {
			for (const keymap* m = *t; m != NULL && s == NULL && m->keysym != TE_KEY_UNDEFINED; m++) {
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
	if (modifiers & TE_MOD_CTRL) {
		int32_t c;
		switch (cp) {
		case ' ':
		case '@':
			c = '\0';	// NUL
			break;
		case '^':
		case '~':
		case '`':
			c = '\036';	// RS
			break;
		default:
			c = -1;
		}
		if (c >= 0) {
			fe_send_back(te, &c, 1);
			return;
		}
	}
	if (modifiers & TE_MOD_META) {
		int32_t buf[] = {'\033', cp, '\0'};
		fe_send_back(te, buf, 2);
	} else {
		// TODO: this little buffer can die
		int32_t buf[] = {cp, '\0'};
		fe_send_back(te, buf, 1);
	}
}

void te_handle_mouse(TE_Backend* te, int mouse_x, int mouse_y, te_mouse_button_t mouse_buttons, te_modifier_t modifiers) {
	// Clamp x,y to within screen
	mouse_x = int_clamp(mouse_x, 0, te->width);
	mouse_y = int_clamp(mouse_y, 0, te->height);

	const bool motion = te->mouse_x != mouse_x || te->mouse_y != mouse_y;
	te->mouse_x = mouse_x;
	te->mouse_y = mouse_y;

	te_mouse_button_t buttonchange = te->mouse_buttons ^ mouse_buttons;
	const te_mouse_button_t pressed = mouse_buttons & buttonchange;					// bitmask, pressed buttons
	const te_mouse_button_t released = (~mouse_buttons) & buttonchange; 			// bitmask, released buttons
	te->mouse_buttons = mouse_buttons & (~(TE_MOUSE_WHEEL_DOWN|TE_MOUSE_WHEEL_UP));	// always release wheel

	// Handle as selection if we're not tracking mouse or shift is held down
	if (te->mouse_mode == MOUSE_TRACK_NONE || modifiers == TE_MOD_SHIFT) {

		if (mouse_buttons & (TE_MOUSE_DOUBLE|TE_MOUSE_TRIPLE)) {
			if (mouse_buttons & TE_MOUSE_TRIPLE) {
				DEBUGF("triple click!\n");
			} else {
				DEBUGF("double click!\n");
			}
		}


		return;
	}

	int32_t buf[] = {'\033', '[', 'M', 0, te->mouse_x+1+32, te->mouse_y+1+32, '\0'};
	const uint8_t base = 32 |	(( modifiers & TE_MOD_CTRL  ) ? 16 : 0) |
								(( modifiers & TE_MOD_META  ) ?  8 : 0) |
								(( modifiers & TE_MOD_SHIFT ) ?  4 : 0);

	// Handle changes in mouse buttons
	if (buttonchange && te->mouse_mode) {
		uint8_t b = base;
		if (pressed & TE_MOUSE_LEFT) {
			b = base;
		} else if (pressed & TE_MOUSE_MIDDLE) {
			b += 1;
		} else if (pressed & TE_MOUSE_RIGHT) {
			b += 2;
		} else if (pressed & TE_MOUSE_WHEEL_UP) {
			b += 64 + 0;
		} else if (pressed & TE_MOUSE_WHEEL_DOWN) {
			b += 64 + 1;
		} else if (released) {
			b += 3;
		} else {
			WARNF("Unexpected button change: %x\n", buttonchange);
			return;
		}
		buf[3] = b;
		fe_send_back(te, buf, 6);
		return;
	}

	// Handle mouse motion
	if (motion && (
		(te->mouse_mode == MOUSE_TRACK_MOTION) ||
		(te->mouse_mode == MOUSE_TRACK_BUTTONMOTION && te->mouse_buttons)
		)
	) {
		uint8_t b = base + 32;	// motion indicator
		if (te->mouse_buttons & TE_MOUSE_LEFT) {
			b = base;
		} else if (te->mouse_buttons & TE_MOUSE_MIDDLE) {
			b += 1;
		} else if (te->mouse_buttons & TE_MOUSE_RIGHT) {
			b += 2;
		} else {
			b += 3;
		}
		buf[3] = b;
		fe_send_back(te, buf, 6);
	}
}

void te_paste_text(TE_Backend* te, const int32_t* data, size_t len) {
	if (be_is_mode_set(te, MODE_BRACKETPASTE)) {
		fe_send_back_char(te, "\033[200~");
		fe_send_back(te, data, len);
		fe_send_back_char(te, "\033[201~");
	} else {
		fe_send_back(te, data, len);
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
	fprintf(where, "Cursor Keys Mode:        %s\n", be_is_mode_set(te, MODE_CURSORAPP) ? "on" : "off");
	fprintf(where, "Application Keypad Mode: %s\n", be_is_mode_set(te, MODE_KEYAPP) ? "on" : "off");
	fprintf(where, "dimensions WxH = %dx%d\n", te->width, te->height);
	fprintf(where, "fg: %d  bg: %d\n", (int)te->fg_color, (int)te->bg_color);
	fprintf(where, "current charset (GR) %d\n", te->charset);
	fprintf(where, "charset g0 us %d uk %d dec %d\n", te->charset_g0==chartable_us, te->charset_g0==chartable_uk, te->charset_g0==chartable_special);
	fprintf(where, "charset g1 us %d uk %d dec %d\n", te->charset_g1==chartable_us, te->charset_g1==chartable_uk, te->charset_g1==chartable_special);
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

