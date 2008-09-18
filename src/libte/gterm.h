// Copyright Timothy Miller, 1999

#ifndef INCLUDED_GTERM_H
#define INCLUDED_GTERM_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "symbol.h"
#include "buffer.h"

#include "libte.h"

#include "macros.h"

#include "gt_typedef.h"

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


struct GTerm_ {

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

GTerm* gterm_new(const TE_Frontend* fe, void* fe_priv, int w, int h);
void gterm_delete(GTerm* gt);

// utility functions
static inline bool gt_is_mode_set(GTerm* gt, te_mode_t mode) {return gt->mode_flags & mode;}
void gt_scroll_region(GTerm* gt, uint start_y, uint end_y, int num);	// does clear
void gt_clear_area(GTerm* gt, int start_x, int start_y, int end_x, int end_y);
void gt_move_cursor(GTerm* gt, int x, int y);

// terminal actions
static inline int gt_get_mode(GTerm* gt) { return gt->mode_flags; }
static inline void gt_set_mode(GTerm* gt, int mode) { gt->mode_flags = mode; }
static inline bool gt_is_mode_flag(GTerm* gt, te_mode_t flag) { return (gt->mode_flags & flag) != 0; }
static inline void gt_set_mode_flag(GTerm* gt, te_mode_t flag) {gt->mode_flags |= flag;}
static inline void gt_clear_mode_flag(GTerm* gt, te_mode_t flag) {gt->mode_flags &= ~flag;}
static inline void gt_clear_mode_flags(GTerm* gt, int flags) {gt->mode_flags &= ~flags;}

// Ordinary printable character are sent here from parser
void gt_input(GTerm* gt, const int32_t* text, size_t len);

void gt_fe_send_back (GTerm* gt, const char* data);
void gt_fe_request_resize (GTerm* gt, int width, int height);
void gt_fe_updated (GTerm* gt);
void gt_fe_move (GTerm* gt, int y, int height, int byoffset);

void gt_process_input(GTerm* gt, int len, const int32_t* data);
void gt_resize_terminal(GTerm* gt, int w, int h);
int gt_handle_button(GTerm* gt, te_key_t key);
void gt_handle_keypress(GTerm* gt, int32_t cp, te_modifier_t modifiers);

#endif

/* End of File */
