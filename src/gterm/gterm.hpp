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

	typedef void (GTerm::*StateFunc)();

	struct StateOption {
		int					cp;		// codepoint value to look for; -1==end/default
		StateFunc			action;
		const StateOption* 	next_state;
	};

#endif

#include "libte.h"

class Buffer;
class Dirty;

class GTerm {
public:

	// mode flags
	typedef enum {
		NOEOLWRAP		= (1<<4),
		CURSORAPPMODE	= (1<<5),
		CURSORRELATIVE	= (1<<6),
		NEWLINE			= (1<<7),
		INSERT			= (1<<8),
		KEYAPPMODE		= (1<<9),
		DEFERUPDATE		= (1<<10),
		DESTRUCTBS		= (1<<11),
		TEXTONLY		= (1<<12),
		LOCALECHO		= (1<<13),
		CURSORINVISIBLE	= (1<<14),
	} mode_t;

	const TE_Frontend*	_fe;
	void*				_fe_priv;

	// terminal info
	int width, height, scroll_top, scroll_bot;
	Buffer*	buffer;
	Dirty* dirty;
	int pending_scroll; // >0 means scroll up
	bool doing_update;

	// terminal state
	int cursor_x, cursor_y;
	int save_x, save_y;
	int fg_color, bg_color;
	symbol_attributes_t	attributes;
	symbol_attributes_t stored_attributes;
	int mode_flags;
	bool* tab_stops;

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
	} parser;

	const StateOption* current_state;

	static StateOption normal_state[];
	static StateOption esc_state[];
	static StateOption bracket_state[];
	static StateOption cset_shiftin_state[];
	static StateOption cset_shiftout_state[];
	static StateOption hash_state[];
	static StateOption vt52_normal_state[];
	static StateOption vt52_esc_state[];
	static StateOption vt52_cursory_state[];
	static StateOption vt52_cursorx_state[];

	void normal_input();

	void set_q_mode();
	void set_quote_mode();
	void clear_param();
	void param_digit();
	void next_param();
	void vt52_cursory();
	void vt52_cursorx();
#endif



	// utility functions
	bool is_mode_set(mode_t mode) {return mode_flags & mode;}
	void scroll_region(int start_y, int end_y, int num);	// does clear
	void shift_text(int y, int start_x, int end_x, int num); // ditto
	void clear_area(int start_x, int start_y, int end_x, int end_y);
	void changed_line(int y, int start_x, int end_x);
	void move_cursor(int x, int y);


	size_t input(const int32_t* text, size_t len);

	// terminal actions

	// non-printing characters
	void cr();
	void lf();
	void ff();
	void bell();
	void tab();
	void bs();

	// escape sequence actions
	void keypad_normal();
	void keypad_application();
	void save_cursor();
	void restore_cursor();
	void set_tab();
	void index_down();
	void index_up();
	void next_line();
	void reset();

	void cursor_left();
	void cursor_down();
	void cursor_right();
	void cursor_up();
	void cursor_position();
	void column_position();
	void line_position();
	void device_attrib();
	void delete_char();
	void set_mode();
	void clear_mode();
	void request_param();
	void set_margins();
	void delete_line();
	void status_report();
	void erase_display();
	void erase_line();
	void insert_line();
	void char_attrs();
	void clear_tab();
	void insert_char();
	void screen_align();
	void erase_char();

	// vt52 stuff
	void vt52_cursor();
	void vt52_ident();

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
	void request_redraw(int x, int y, int w, int h, bool force);
	int get_width() { return width; }
	int get_height() { return height; }
};

#endif

/* End of File */
