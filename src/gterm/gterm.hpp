// Copyright Timothy Miller, 1999

#ifndef INCLUDED_GTERM_H
#define INCLUDED_GTERM_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

//#define USE_VTPARSE

#include "buffersymbol.h"


#ifdef USE_VTPARSE

#	include "vtparse.h"

#else

	class GTerm;

	typedef void (*StateFunc)(GTerm* gt);

	struct StateOption {
		int							cp;			// codepoint value to look for; -1==end/default
		StateFunc					action;		// action to execute on this transition
		const struct StateOption* 	next_state;	// state to transfer to next
	};

#endif

#include "libte.h"

class Buffer;
class Dirty;

class GTerm {
public:

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
		AUTOWRAP		= (1<<4),

		CURSORAPPMODE	= (1<<5),
		CURSORRELATIVE	= (1<<6),

		// New line mode (DECNLM)
		//
		// With new line:
		//   Causes a received linefeed, form feed, or vertical tab to move cursor to first column of next line.
		//   RETURN transmits both a carriage return and linefeed. This selection is also called new line option.
		//
		// Without new line:
		//   Causes a received linefeed, form feed, or vertical tab to move cursor to next line in current column.
		//   RETURN transmits a carriage return.
		NEWLINE			= (1<<7),

		// Insertion-Replacement Mode (IRM)
		// With insert mode:
		//   New display characters move old display characters to the right.
		//   Characters moved past the right margin are lost.
		// Without insert mode:
		//   New display characters replace old display characters at cursor position.
		//   The old character is erased.
		INSERT			= (1<<8),

		// Application Keypad Mode (DECKPAM)
		KEYAPPMODE		= (1<<9),

		// Scroll Mode (DECSCLM)
		//
		// With scroll mode:
		//   Smooth scroll lets the terminal add 6 lines per second to screen (power feature = 60 Hz),
		//   or 5 lines per second (power feature = 50 Hz).
		// Without scroll mode:
		//   Jump scroll lets the terminal add lines to the screen as fast as possible.
		SMOOTHSCROLL	= (1<<10),

		DESTRUCTBS		= (1<<11),

		// TODO: what is this??
		TEXTONLY		= (1<<12),

		// Send-Receive Mode (SRM), local echo
		LOCALECHO		= (1<<13),

		CURSORINVISIBLE	= (1<<14),

		// Origin mode (DECOM)
		// TODO: Not supported yet
		//
		// With origin mode:
		//   Selects home position in scrolling region. Line numbers start at top margin of
		//   scrolling region. The cursor cannot move out of scrolling region.
		//
		// Without origin mode [DEFAULT]:
		//   Selects home position in upper-left corner of screen. Line numbers are independent
		//   of the scrolling region (absolute). Use CUP and HVP sequences to move cursor out of
		//   scrolling region.
		ORIGIN			= (1<<15),
	} mode_t;

	// Contains the currently set mode flags
	int mode_flags;

	// An boolean array where true means that there exists a tab-stop at that column
	bool* tab_stops;

	const TE_Frontend*	_fe;
	void*				_fe_priv;

	// terminal info
	int width, height;
	Buffer*	buffer;
	Dirty* dirty;

	// Scroll margins, as set by DECSTBM
	int scroll_top, scroll_bot;

	int pending_scroll; // >0 means scroll up
	bool doing_update;

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

#ifdef USE_VTPARSE
	vtparse_t parser;
#else

	struct {
		// action parameters
		int num_params;
		int params[16];

		unsigned char intermediate_chars[2];

		const int32_t*	input_data;
		size_t			input_remaining;

		const StateOption* current_state;
	} parser;
#endif



	// utility functions
	bool is_mode_set(mode_t mode) {return mode_flags & mode;}
	void scroll_region(int start_y, int end_y, int num);	// does clear
	void shift_text(int y, int start_x, int end_x, int num); // ditto
	void clear_area(int start_x, int start_y, int end_x, int end_y);
	void changed_line(int y, int start_x, int end_x);
	void move_cursor(int x, int y);


	// Ordinary printable character are sent here from parser
	void input(const int32_t* text, size_t len);

	// terminal actions

	int get_mode() { return mode_flags; }
	void set_mode(int mode) { mode_flags = mode; }
	bool is_mode_flag(mode_t flag) { return (mode_flags & flag) != 0; }
	void set_mode_flag(mode_t flag) {mode_flags |= flag;}
	void clear_mode_flag(mode_t flag) {mode_flags &= ~flag;}
	void clear_mode_flags(int flags) {mode_flags &= ~flags;}

	void fe_send_back (const char* data);
	void fe_request_resize (int width, int height);
	void fe_updated (void);
	void fe_scroll (int y, int height, int offset);

	GTerm(const TE_Frontend* fe, void* fe_priv, int w, int h);
	GTerm(const GTerm& old);
	virtual ~GTerm();


	void process_input(int len, const int32_t* data);
	void update_changes(void);
	void resize_terminal(int w, int h);
	void handle_button(te_key_t key);
	void handle_keypress(int32_t cp, te_modifier_t modifiers);
	void request_redraw(int x, int y, int w, int h, bool force);
	int get_width() { return width; }
	int get_height() { return height; }
};

#endif

/* End of File */
