/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef INCLUDED_GTERM_H
#define INCLUDED_GTERM_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "symbol.h"
#include "buffer.h"

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
	MODE_AUTOWRAP	= (1<<4),

	MODE_CURSORAPP	= (1<<5),

	// New line mode (DECNLM)
	//
	// With new line:
	//   Causes a received linefeed, form feed, or vertical tab to move cursor to first column of next line.
	//   RETURN transmits both a carriage return and linefeed. This selection is also called new line option.
	//
	// Without new line:
	//   Causes a received linefeed, form feed, or vertical tab to move cursor to next line in current column.
	//   RETURN transmits a carriage return.
	MODE_NEWLINE	= (1<<7),

	// Insertion-Replacement Mode (IRM)
	// With insert mode:
	//   New display characters move old display characters to the right.
	//   Characters moved past the right margin are lost.
	// Without insert mode:
	//   New display characters replace old display characters at cursor position.
	//   The old character is erased.
	MODE_INSERT		= (1<<8),

	// Application Keypad Mode (DECKPAM)
	MODE_KEYAPP		= (1<<9),

	// Scroll Mode (DECSCLM)
	//
	// With scroll mode:
	//   Smooth scroll lets the terminal add 6 lines per second to screen (power feature = 60 Hz),
	//   or 5 lines per second (power feature = 50 Hz).
	// Without scroll mode:
	//   Jump scroll lets the terminal add lines to the screen as fast as possible.
	MODE_SMOOTHSCROLL	= (1<<10),

	// TODO: what are these??
	MODE_DESTRUCTBS		= (1<<11),
	MODE_TEXTONLY		= (1<<12),

	// Send-Receive Mode (SRM), local echo
	MODE_LOCALECHO		= (1<<13),

	MODE_CURSORINVISIBLE	= (1<<14),

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
	MODE_ORIGIN			= (1<<15),
} te_mode_t;


struct TE_Backend_ {

	// Contains the currently set mode flags
	int mode_flags;

	// An boolean array where true means that there exists a tab-stop at that column
	bool* tab_stops;

	const TE_Frontend*	fe;
	void*				fe_priv;

	// terminal info
	int width, height;
	Buffer	buffer;
	History	history;


	// Scroll margins, as set by DECSTBM
	int scroll_top, scroll_bot;

	// terminal state
	int cursor_x, cursor_y;

	symbol_color_t fg_color;
	symbol_color_t bg_color;
	symbol_attributes_t	attributes;


	// Used by store-cursor / restore-cursor (DECSC/DECRC)
	struct {
		int cursor_x, cursor_y;
		symbol_attributes_t attributes;
		bool autowrap;
	} stored;

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

static inline void fe_send_back		(TE* te, const int32_t* data)
	{ te->fe->send_back(te->fe_priv, data); }

static inline void fe_bell			(TE* te)
	{ te->fe->bell(te->fe_priv); }

static inline void fe_request_resize	(TE* te, int width, int height)
	{ te->fe->request_resize(te->fe_priv, width, height); }

static inline void fe_updated		(TE* te)
	{ te->fe->updated(te->fe_priv); }

static inline void fe_move			(TE* te, int y, int height, int byoffset)
	{	te->fe->draw_move(te->fe_priv, y, height, byoffset); }

static inline void fe_position		(TE* te, int offset, int size)
	{ te->fe->position(te->fe_priv, offset, size); }

static inline void fe_draw_text		(TE* te, int x, int y, const symbol_t* symbols, int len)
	{ te->fe->draw_text(te->fe_priv, x, y, symbols, len); }

static inline void fe_draw_clear	(TE* te, int x, int y, const symbol_color_t bg_color, int len)
	{ te->fe->draw_clear(te->fe_priv, x, y, bg_color, len); }

static inline void fe_draw_cursor	(TE* te, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp)
	{ te->fe->draw_cursor(te->fe_priv, fg_color, bg_color, attrs, x, y, cp); }

static inline void fe_draw_move		(TE* te, int y, int height, int byoffset)
	{te->fe->draw_move(te->fe_priv, y, height, byoffset); }




void	be_process_input(TE* te, int len, const int32_t* data);
void	be_resize_terminal(TE* te, int w, int h);
int		be_handle_button(TE* te, te_key_t key);
void	be_handle_keypress(TE* te, int32_t cp, te_modifier_t modifiers);

#endif

/* End of File */
