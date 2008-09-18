// Copyright Timothy Miller, 1999

#ifndef INCLUDED_GTERM_H
#define INCLUDED_GTERM_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "symbol.h"
#include "Buffer.h"

#include "states.h"

#include "libte.h"


#include "Dirty.h"

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


class GTerm {
public:

	// Contains the currently set mode flags
	int mode_flags;

	// An boolean array where true means that there exists a tab-stop at that column
	bool* tab_stops;

	const TE_Frontend*	_fe;
	void*				_fe_priv;

	// terminal info
	int width, height;
	Buffer	buffer;
	History	history;

	struct {
		uint	offset;
		Dirty	dirty;
		bool	updating;
		bool	scroll_lock;
	} viewport;

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

	struct {
		// action parameters
		int num_params;
		int params[16];

		unsigned char intermediate_chars[2];

		const int32_t*	input_data;
		size_t			input_remaining;

		const StateOption* current_state;
	} parser;

	// Ordinary printable character are sent here from parser
	void input(const int32_t* text, size_t len);

	// terminal actions

	int get_mode() { return mode_flags; }
	void set_mode(int mode) { mode_flags = mode; }
	bool is_mode_flag(te_mode_t flag) { return (mode_flags & flag) != 0; }
	void set_mode_flag(te_mode_t flag) {mode_flags |= flag;}
	void clear_mode_flag(te_mode_t flag) {mode_flags &= ~flag;}
	void clear_mode_flags(int flags) {mode_flags &= ~flags;}

	void fe_send_back (const char* data);
	void fe_request_resize (int width, int height);
	void fe_updated (void);
	void fe_move (int y, int height, int byoffset);

	GTerm(const TE_Frontend* fe, void* fe_priv, int w, int h);
	GTerm(const GTerm& old);
	virtual ~GTerm();

	void process_input(int len, const int32_t* data);
	void update_changes(void);
	void resize_terminal(int w, int h);
	int handle_button(te_key_t key);
	void handle_keypress(int32_t cp, te_modifier_t modifiers);
};

// utility functions
static inline bool gt_is_mode_set(GTerm* gt, te_mode_t mode) {return gt->mode_flags & mode;}
void gt_scroll_region(GTerm* gt, uint start_y, uint end_y, int num);	// does clear
void gt_clear_area(GTerm* gt, int start_x, int start_y, int end_x, int end_y);
void gt_changed_line(GTerm* gt, int y, int start_x, int end_x);
void gt_move_cursor(GTerm* gt, int x, int y);


#endif

/* End of File */
