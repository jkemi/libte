/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "symbol.h"
#include "buffer.h"
#include "charsets.h"

#include "libte.h"

#include "macros.h"

#include "typedef.h"

// mode flags
typedef enum {
	// Auto wrap mode (DECAWM)
	//
	// With auto wrap:
	//   Any display characters received when cursor is at right margin appear on next line.
	//   The display scrolls up if cursor is at end of scrolling region.
	// Without auto wrap:
	//   Display characters received when cursor is at right margin replace previously
	//   displayed character.
	MODE_AUTOWRAP	= (1<<0),

	// Cursor Keys Mode (DECCKM)
	// This control function selects the sequences the arrow keys send.
	// You can use the four arrow keys to move the cursor through the
	// current page or to send special application commands.
	// With cursor keys mode:
	//   The arrow keys send application sequences to the host.
	// Without cursor keys mode:
	//   The arrow keys send ANSI cursor sequences to the host.
	MODE_CURSORAPP	= (1<<1),

	// New line mode (DECNLM)
	//
	// With new line:
	//   Causes a received linefeed, form feed, or vertical tab to move cursor to first column of next line.
	//   RETURN transmits both a carriage return and linefeed. This selection is also called new line option.
	//
	// Without new line:
	//   Causes a received linefeed, form feed, or vertical tab to move cursor to next line in current column.
	//   RETURN transmits a carriage return.
	MODE_NEWLINE	= (1<<2),

	// Insertion-Replacement Mode (IRM)
	// With insert mode:
	//   New display characters move old display characters to the right.
	//   Characters moved past the right margin are lost.
	// Without insert mode:
	//   New display characters replace old display characters at cursor position.
	//   The old character is erased.
	MODE_INSERT		= (1<<3),

	// Application Keypad Mode (DECKPAM)
	MODE_KEYAPP		= (1<<4),

	// Scroll Mode (DECSCLM)
	//
	// With scroll mode:
	//   Smooth scroll lets the terminal add 6 lines per second to screen (power feature = 60 Hz),
	//   or 5 lines per second (power feature = 50 Hz).
	// Without scroll mode:
	//   Jump scroll lets the terminal add lines to the screen as fast as possible.
	MODE_SMOOTHSCROLL	= (1<<5),

	// TODO: what are these??
	MODE_DESTRUCTBS		= (1<<6),
	MODE_TEXTONLY		= (1<<7),

	// Send-Receive Mode (SRM), local echo
	MODE_LOCALECHO		= (1<<8),

	MODE_CURSORINVISIBLE	= (1<<9),

	// Origin mode (DECOM)
	//
	// With origin mode:
	//   Selects home position in scrolling region. Line numbers start at top margin of
	//   scrolling region. The cursor cannot move out of scrolling region.
	//
	// Without origin mode [DEFAULT]:
	//   Selects home position in upper-left corner of screen. Line numbers are independent
	//   of the scrolling region (absolute). Use CUP and HVP sequences to move cursor out of
	//   scrolling region.
	MODE_ORIGIN			= (1<<10),

	// Screen mode (DECSCNM)
	//
	// Set selects reverse screen, a white screen background with black characters.
	// reverses default fg/bg
	MODE_SCREEN			= (1<<11),

	// Bracketed paste mode (xterm)
	//
	// When bracketed paste mode is set, pasted text is bracketed with control sequences so
	// that the program can differentiate pasted text from typed-in text.
	// When bracketed paste mode is set, the program will receive: ESC [ 200 ~, followed by the
	// pasted text, followed by ESC [ 201 ~.
	MODE_BRACKETPASTE	= (1<<12),

} te_mode_t;

typedef enum {
	MOUSE_TRACK_NONE			= 0,	// No mouse tracking (default)
	MOUSE_TRACK_BUTTON			= 1,	// Report button press and release (xterm: VT200_MOUSE)
	MOUSE_TRACK_BUTTONMOTION	= 2,	// Report mouse motion when any button is pressed (xterm: BTN_EVENT_MOUSE)
	MOUSE_TRACK_MOTION			= 3,	// Report mouse motion without any button pressed (xterm: ANY_EVENT_MOUSE)
} te_mouse_mode_t;

struct TE_Backend_ {

	// Contains the currently set mode flags
	int mode_flags;

	// A boolean array where true means that there exists a tab-stop at that column
	bool* tab_stops;

	const TE_Frontend*	fe;
	void*				fe_priv;

	// terminal info
	int width, height;
	Buffer*	buffer;
	History* history;

	Buffer	norm_buffer;
	History	norm_history;
	Buffer	alt_buffer;
	History	alt_history;

	// Scroll margins, as set by DECSTBM
	int scroll_top, scroll_bot;

	// terminal state
	int cursor_x, cursor_y;

	symbol_color_t fg_color;
	symbol_color_t bg_color;
	symbol_attributes_t	attributes;

	int charset;			// (GL)
	const te_chartable_entry_t* charset_g0;
	const te_chartable_entry_t* charset_g1;

	// Used by store-cursor / restore-cursor (DECSC/DECRC)
	struct {
		int cursor_x, cursor_y;
		symbol_color_t	fg;
		symbol_color_t	bg;
		symbol_attributes_t attributes;
		bool autowrap;
		int charset;
		const te_chartable_entry_t* charset_g0;
		const te_chartable_entry_t* charset_g1;
	} stored;

	int					mouse_x, mouse_y;	// last reported mouse pos.
	te_mouse_button_t	mouse_buttons;		// last reported mouse button status.
	te_mouse_mode_t		mouse_mode;

	enum {
		SELSTATE_NONE,
		SELSTATE_MARKING,
		SELSTATE_MARKED,
	} selstate;

	int selstart_x, selstart_y;
	int selend_x, selend_y;
	int selpos_x, selpos_y;		// used with SELSTATE_MARKING

	te_color_t palette[TE_COLOR_COUNT];

	struct Parser_* parser;
	struct Viewport_* viewport;
};

TE* te_new(const TE_Frontend* fe, void* fe_priv, int w, int h);
void be_delete(TE* te);

// utility functions
static inline bool be_is_mode_set(TE* te, int mode) { return te->mode_flags & mode; }
void be_scroll_region(TE* te, uint start_y, uint end_y, int num);	// does clear
void be_clear_area(TE* te, int start_x, int start_y, int end_x, int end_y);
void be_move_cursor(TE* te, int x, int y);
void be_switch_buffer(TE* te, bool alt, bool erase_on_alt);
void be_screen_mode(TE* te, bool enable);

// terminal actions
static inline int be_get_mode(TE* te) { return te->mode_flags; }
static inline void be_set_mode(TE* te, int mode) { te->mode_flags = mode; }
static inline bool be_is_mode_flag(TE* te, te_mode_t flag) { return (te->mode_flags & flag) != 0; }
static inline void be_set_mode_flag(TE* te, te_mode_t flag) {te->mode_flags |= flag;}
static inline void be_clear_mode_flag(TE* te, te_mode_t flag) {te->mode_flags &= ~flag;}
static inline void be_clear_mode_flags(TE* te, int flags) {te->mode_flags &= ~flags;}

// Ordinary printable character are sent here from parser
void be_input(TE* te, const int32_t* text, size_t len);

void fe_send_back_char (TE* te, const char* data);

static inline void fe_send_back		(TE* te, const int32_t* data, int len)
	{ te->fe->send_back(te->fe_priv, data, len); }

static inline void fe_bell			(TE* te)
	{ te->fe->bell(te->fe_priv); }

static inline void fe_request_resize	(TE* te, int width, int height)
	{ te->fe->request_resize(te->fe_priv, width, height); }

static inline void fe_title	(TE* te, const int32_t* text, int len)
	{ te->fe->title(te->fe_priv, text, len); }

static inline void fe_updated		(TE* te)
	{ te->fe->updated(te->fe_priv); }

static inline void fe_position		(TE* te, int offset, int size)
	{ te->fe->position(te->fe_priv, offset, size); }

static inline void fe_palette	(TE* te, int offset, int count, const te_color_t* data)
	{ te->fe->palette(te->fe_priv, offset, count, data); }

static inline void fe_draw_text		(TE* te, int x, int y, const symbol_t* symbols, int len)
	{ te->fe->draw_text(te->fe_priv, x, y, symbols, len); }

static inline void fe_draw_clear	(TE* te, int x, int y, const symbol_color_t bg_color, int len)
	{ te->fe->draw_clear(te->fe_priv, x, y, bg_color, len); }

static inline void fe_draw_cursor	(TE* te, int x, int y, symbol_t symbol)
	{ te->fe->draw_cursor(te->fe_priv, x, y, symbol); }

static inline void fe_draw_move		(TE* te, int y, int height, int byoffset)
	{ te->fe->draw_move(te->fe_priv, y, height, byoffset); }



void	be_process_input(TE* te, int len, const int32_t* data);
void	be_resize_terminal(TE* te, int w, int h);
int		be_handle_button(TE* te, te_key_t key);
void	be_handle_keypress(TE* te, int32_t cp, te_modifier_t modifiers);

#endif // INTERNAL_H_
