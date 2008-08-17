// Copyright Timothy Miller, 1999

//
// Methods called from state machine
//

#include "gterm.hpp"

#include "Buffer.h"
#include "misc.h"

static inline int _get_param(GTerm* gt, int index, int default_value) {
	if (gt->parser.num_params > index) {
		return gt->parser.params[index];
	} else {
		return default_value;
	}
}

void GTerm::cr()
{
	move_cursor(0, cursor_y);
}

void GTerm::lf()
{
	if (cursor_y < scroll_bot) {
		move_cursor(cursor_x, cursor_y+1);
	} else {
		scroll_region(scroll_top, scroll_bot, 1);
	}
}

void GTerm::ff()
{
	clear_area(0, scroll_top, width-1, scroll_bot);
	move_cursor(0, scroll_top);
}

void GTerm::tab()
{
	int x = -1;

	// find nearest set tab-stop
	for (int i = cursor_x+1; i < width; i++) {
		if (tab_stops[i]) {
			x = i;
			break;
		}
	}

	// set tab-stop not found, use 8 spaces
	if (x < 0) {
		x = (cursor_x+7) & ~0x7;
	}

	if (x < width) {
		move_cursor(x, cursor_y);
	} else {
		if (is_mode_set(NOEOLWRAP)) {
			move_cursor(width-1, cursor_y);
		} else {
			next_line();
		}
	}
}

void GTerm::bs()
{
	if (cursor_x > 0) {
		move_cursor(cursor_x-1, cursor_y);
	}
	if (is_mode_set(DESTRUCTBS)) {
	    clear_area(cursor_x, cursor_y, cursor_x, cursor_y);
	}
}

void GTerm::bell()
{
	if (_fe->bell != NULL) {
		_fe->bell(_fe_priv);
	}
}

void GTerm::keypad_normal() { clear_mode_flag(KEYAPPMODE); }
void GTerm::keypad_application() { set_mode_flag(KEYAPPMODE); }

// Save cursor (ANSI.SYS)
void GTerm::save_cursor()
{
	stored_attributes = attributes;
	save_x = cursor_x;
	save_y = cursor_y;
}

void GTerm::restore_cursor()
{
	attributes = stored_attributes;
	move_cursor(save_x, save_y);
}

void GTerm::set_tab()
{
	tab_stops[cursor_x] = true;
}

void GTerm::index_down()
{
	lf();
}

void GTerm::next_line()
{
	lf();
	cr();
}

void GTerm::index_up()
{
	if (cursor_y > scroll_top) {
		move_cursor(cursor_x, cursor_y-1);
	} else {
		scroll_region(scroll_top, scroll_bot, -1);
	}
}

void GTerm::reset()
{
	pending_scroll = 0;
	bg_color = 0;
	fg_color = 7;
	scroll_top = 0;
	scroll_bot = height-1;
	memset(tab_stops, 0, sizeof(bool)*width);

	clear_mode_flags(NOEOLWRAP | CURSORAPPMODE | CURSORRELATIVE | KEYAPPMODE | CURSORINVISIBLE);

	attributes = 0;

	clear_area(0, 0, width, height-1);
	move_cursor(0, 0);
}

// Cursor Backward P s Times (default = 1) (CUB)
void GTerm::cursor_left()
{
	int n, x;
	n = int_max(1, _get_param(this,0,1) );

	x = int_max(0, cursor_x-n);
	move_cursor(x, cursor_y);
}

// Cursor Forward P s Times (default = 1) (CUF)
void GTerm::cursor_right()
{
	int n, x;
	n = int_max(1, _get_param(this,0,1) );

	x = int_min(width-1, cursor_x+n);
	move_cursor(x, cursor_y);
}

// Cursor Up P s Times (default = 1) (CUU)
void GTerm::cursor_up()
{
	int n, y;
	n = int_max(1, _get_param(this,0,1) );

	y = int_max(0, cursor_y-n);;
	move_cursor(cursor_x, y);
}

// Cursor Down P s Times (default = 1) (CUD)
void GTerm::cursor_down()
{
	int n, y;
	n = int_max(1, _get_param(this,0,1) );

	y = int_min(height-1, cursor_y+n);
	move_cursor(cursor_x, y);
}

void GTerm::cursor_position()
{
	int	y = int_clamp(_get_param(this, 0, 1), 1, height);
	int x = int_clamp(_get_param(this, 1, 1), 1, width);

	if (is_mode_set(CURSORRELATIVE)) {
		move_cursor(x-1, y-1+scroll_top);
	} else {
		move_cursor(x-1, y-1);
	}
}

// Cursor Character Absolute [column] (default = [row,1]) (CHA)
void GTerm::column_position()
{
	int	x = int_clamp(_get_param(this, 0, 1), 1, width);

	move_cursor(x-1, cursor_y);
}

// Line Position Absolute [row] (default = [1,column]) (VPA)
void GTerm::line_position()
{
	int	y = int_clamp(_get_param(this, 0, 1), 1, height);

	move_cursor(cursor_x, y-1);
}


void GTerm::device_attrib()
{
	fe_send_back("\033[?1;2c");
}

// Delete P s Character(s) (default = 1) (DCH)
void GTerm::delete_char()
{
	int n, mx;
	n = int_max(1, _get_param(this,0,1) );

	mx = width-cursor_x;
	if (n >= mx) {
		clear_area(cursor_x, cursor_y, width-1, cursor_y);
	} else {
		shift_text(cursor_y, cursor_x, width-1, -n);
	}
}

// Set Mode (SM)
void GTerm::set_mode()  // h
{
	if (parser.intermediate_chars[0] == '?') {
		// DEC Private Mode Set (DECSET)
		// Lots of these are missing

		switch (_get_param(this,0,-1)) {
		case 1:	set_mode_flag(CURSORAPPMODE);	break;	// Normal Cursor Keys (DECCKM)
//		case 2:											// Designate VT52 mode (DECANM).
		case 3:	fe_request_resize(132, height);	break;	// 80 Column Mode (DECCOLM)
		case 6:	set_mode_flag(CURSORRELATIVE);	break;	// Normal Cursor Mode (DECOM)
		case 7:	clear_mode_flag(NOEOLWRAP);		break;	// No Wraparound Mode (DECAWM)
		case 25:										// Hide Cursor (DECTCEM)
			clear_mode_flag(CURSORINVISIBLE);
			move_cursor(cursor_x, cursor_y);
			break;
		default:
			break;
		}
	} else {
		// Set Mode (SM)

		switch (_get_param(this,0,-1)) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		set_mode_flag(INSERT);  	break;	// Insert Mode (IRM)
		case 12:	clear_mode_flag(LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	set_mode_flag(NEWLINE);		break;	// Automatic Newline (LNM)
		default:
			break;
		}
	}
}

// Reset Mode (RM)
void GTerm::clear_mode()  // l
{
	if (parser.intermediate_chars[0] == '?') {
		// DEC Private Mode Reset (DECRST)
		// Lots of these are missing

		switch (_get_param(this,0,-1)) {
		case 1:	clear_mode_flag(CURSORAPPMODE);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:	current_state = vt52_normal_state; break;	// Designate VT52 mode (DECANM).
		case 3:	fe_request_resize(80, height);		break;	// 80 Column Mode (DECCOLM)
		case 6:	clear_mode_flag(CURSORRELATIVE);	break;	// Normal Cursor Mode (DECOM)
		case 7:	set_mode_flag(NOEOLWRAP);			break;	// No Wraparound Mode (DECAWM)
		case 25:											// Hide Cursor (DECTCEM)
			set_mode_flag(CURSORINVISIBLE);	break;
			move_cursor(cursor_x, cursor_y);
			break;
		}
	} else {
		// Reset Mode (RM)

		switch (_get_param(this,0,-1)) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		clear_mode_flag(INSERT);	break;	// Insert Mode (IRM)
		case 12:	set_mode_flag(LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	clear_mode_flag(NEWLINE);	break;	// Automatic Newline (LNM)
		}
	}
}

void GTerm::request_param()
{
	char str[40];
	sprintf(str, "\033[%d;1;1;120;120;1;0x", parser.params[0]+2);

	fe_send_back(str);
}

void GTerm::set_margins()
{
	int t, b;

	t = int_max(1, parser.params[0]);
	b = int_clamp(parser.params[1], 1, height);

	if (pending_scroll) {
		update_changes();
	}

	scroll_top = t-1;
	scroll_bot = b-1;
	if (cursor_y < scroll_top) {
		move_cursor(cursor_x, scroll_top);
	} else if (cursor_y > scroll_bot) {
		move_cursor(cursor_x, scroll_bot);
	}
}

void GTerm::delete_line()
{
	int n, mx;
	n = int_max(1, parser.params[0]);
	mx = scroll_bot-cursor_y+1;
	if (n>=mx) {
		clear_area(0, cursor_y, width-1, scroll_bot);
	} else {
		scroll_region(cursor_y, scroll_bot, n);
	}
}

void GTerm::status_report()
{
	char str[20];
	switch (parser.params[0]) {
	case 5:
		fe_send_back("\033[0n");
		break;
	case 6:
		sprintf(str, "\033[%d;%dR", cursor_y+1, cursor_x+1);
		fe_send_back(str);
		break;
	}
}

// Erase in Display (ED)
void GTerm::erase_display()
{
	switch ( _get_param(this, 0, 0) ) {
	case 0:
		clear_area(cursor_x, cursor_y, width-1, cursor_y);
		if (cursor_y<height-1)
			clear_area(0, cursor_y+1, width-1, height-1);
		break;
	case 1:
		clear_area(0, cursor_y, cursor_x, cursor_y);
		if (cursor_y>0)
			clear_area(0, 0, width-1, cursor_y-1);
		break;
	case 2:
		clear_area(0, 0, width-1, height-1);
		break;
	}
}

void GTerm::erase_line()
{
	switch ( _get_param(this, 0, 0) ) {
	case 0:	// Erase to Right (default)
		clear_area(cursor_x, cursor_y, width, cursor_y);
		break;
	case 1:	// Erase to Left
		clear_area(0, cursor_y, cursor_x, cursor_y);
		break;
	case 2: // Erase All
		clear_area(0, cursor_y, width, cursor_y);
		break;
	}
}


void GTerm::insert_line()
{
	int n, mx;
	n = int_max(1, parser.params[0]);
	mx = scroll_bot-cursor_y+1;
	if (n>=mx) {
		clear_area(0, cursor_y, width-1, scroll_bot);
	} else {
		scroll_region(cursor_y, scroll_bot, -n);
	}
}

// Character Attributes (SGR)
void GTerm::char_attrs()
{
	// TODO: REWRITE THIS METHOD

	int n;

	// TODO: what?? this if case seems wrong
	if (parser.num_params == 0 || parser.params[0] == 0) {
		attributes = 0;
		fg_color = 7;
		bg_color = 0;
		return;
	}

//	attributes = 0; 	// imm... linux console seems to leave underline on. This "fixes" that...
	/// TODO: hmm, should probably be < not <=
	for (n = 0; n < parser.num_params; n++) {
		const int p = parser.params[n];
		if (p/10 == 4) {
			bg_color = int_clamp(p%10, 0, 7);
		} else if (p/10 == 3) {
			fg_color = int_clamp(p%10, 0, 7);
		} else {
			switch (p) {
			case 0:
				attributes = 0;
				fg_color = 7;
				bg_color = 0;
				break;
			case 1: attributes |= SYMBOL_BOLD; break;
			case 4: attributes |= SYMBOL_UNDERLINE; break;
			case 5:	attributes |= SYMBOL_BLINK; break;
			case 7:	attributes |= SYMBOL_INVERSE; break;
			}
		}
	}
}

void GTerm::clear_tab()
{
	switch (parser.params[0]) {
	case 3:
		memset(tab_stops, 0, sizeof(tab_stops));
		break;
	case 0:
		tab_stops[cursor_x] = false;
	}
}

void GTerm::insert_char()
{
	int n, mx;
	n = int_max(1, parser.params[0]);
	mx = width-cursor_x;
	if (n >= mx) {
		clear_area(cursor_x, cursor_y, width-1, cursor_y);
	} else {
		shift_text(cursor_y, cursor_x, width-1, n);
	}
}

void GTerm::screen_align()
{
	const symbol_t style = symbol_make_style(7,0,0);
	const symbol_t sym = 'E' | style;

	// TODO: fixup
	symbol_t syms[width];
	for (int x = 0; x < width; x++) {
		syms[x] = sym;
	}

	for (int y=0; y<height; y++) {
		BufferRow* row = buffer->getRow(y);
		changed_line(y, 0, width-1);
		row->replace(0, syms, width);
	}
}

// Erase P s Character(s) (default = 1) (ECH)
void GTerm::erase_char()
{
	// number of characters to erase
	const int n = int_clamp(_get_param(this,0,1), 1, width-cursor_x);
	clear_area(cursor_x, cursor_y, cursor_x+n-1, cursor_y);
}

void GTerm::vt52_cursor() {
	const int y = int_clamp(parser.params[0]-32, 1, height);
	const int x = int_clamp(parser.params[1]-32, 1, width);

	move_cursor(x-1, y-1);
}

void GTerm::vt52_ident()
{
	fe_send_back("\033/Z");
}
