// Copyright Timothy Miller, 1999

#include "gterm.hpp"

#include "Buffer.h"
#include "misc.h"

// For efficiency, this grabs all printing characters from buffer, up to
// the end of the line or end of buffer
void GTerm::normal_input()
{
	if (*input_data < 32) {
		return;
	}

	// this check doesn't work and should probably not be here to begin with...
	if (cursor_x >= width) {
		if (is_mode_set(NOEOLWRAP)) {
			cursor_x = width-1;
		} else {
			next_line();
		}
	}

	// Count number of consumed bytes and number of bytes to print
	int n = 0;		// number of bytes to draw
	int n_taken;	// number of bytes consumed from input data stream
	if (is_mode_set(NOEOLWRAP)) {
		while (n < input_remaining && input_data[n] > 31) {
			n++;
		}
		n_taken = n;
		if (n >= width-cursor_x) {
			n = width-cursor_x;
		}
	} else {
		const int sz = int_min(input_remaining, width-cursor_x);
		while (n < sz && input_data[n] > 31) {
			n++;
		}
		n_taken = n;
	}

	// TODO: remove temporary stack buffer from here..
	symbol_t syms[n];
	symbol_t style = symbol_make_style(fg_color, bg_color, attributes);
	for (int i = 0; i < n; i++) {
		const symbol_t sym = style | input_data[i];
		syms[i] = sym;
	}
	BufferRow* row = buffer->getRow(cursor_y);

	if (is_mode_set(INSERT)) {
		row->insert(cursor_x, syms, n);
		changed_line(cursor_y, cursor_x, width);
	} else {
		row->replace(cursor_x, syms, n);
		changed_line(cursor_y, cursor_x, cursor_x+n);
	}

	cursor_x += n;

	// TODO: why -1 ??
	input_data += n_taken-1;
	input_remaining -= n_taken-1;
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
	Bell();
}

void GTerm::clear_param()
{
	nparam = 0;
	memset(param, 0, sizeof(param));
	q_mode = 0;
	got_param = false;
}

void GTerm::keypad_numeric() { clear_mode_flag(KEYAPPMODE); }
void GTerm::keypad_application() { set_mode_flag(KEYAPPMODE); }

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
	current_state = GTerm::normal_state;

	clear_mode_flags(NOEOLWRAP | CURSORAPPMODE | CURSORRELATIVE | KEYAPPMODE | CURSORINVISIBLE);
	attributes = 0;

	clear_area(0, 0, width, height-1);
	move_cursor(0, 0);
}

void GTerm::set_q_mode()
{
	q_mode = 1;
}

// The verification test used some strange sequence which was
// ^[[61"p
// in a function called set_level,
// but it didn't explain the meaning.  Just in case I ever find out,
// and just so that it doesn't leave garbage on the screen, I accept
// the quote and mark a flag.
void GTerm::set_quote_mode()
{
	quote_mode = 1;
}

// for performance, this grabs all digits
void GTerm::param_digit()
{
	got_param = true;
	param[nparam] = param[nparam]*10 + (*input_data)-'0';
}

void GTerm::next_param()
{
	nparam++;
}

void GTerm::cursor_left()
{
	int n, x;
	n = param[0]; if (n<1) n = 1;
	x = cursor_x-n; if (x<0) x = 0;
	move_cursor(x, cursor_y);
}

void GTerm::cursor_right()
{
	int n, x;
	n = param[0]; if (n<1) n = 1;
	x = cursor_x+n; if (x>=width) x = width-1;
	move_cursor(x, cursor_y);
}

void GTerm::cursor_up()
{
	int n, y;
	n = param[0]; if (n<1) n = 1;
	y = cursor_y-n; if (y<0) y = 0;
	move_cursor(cursor_x, y);
}

void GTerm::cursor_down()
{
	int n, y;
	n = param[0]; if (n<1) n = 1;
	y = cursor_y+n; if (y>=height) y = height-1;
	move_cursor(cursor_x, y);
}

void GTerm::cursor_position()
{
	int x, y;
	x = param[1];	if (x<1) x=1; if (x>=width) x = width-1;
	y = param[0];	if (y<1) y=1; if (y>=height) y = height-1;
	if (is_mode_set(CURSORRELATIVE)) {
		move_cursor(x-1, y-1+scroll_top);
	} else {
		move_cursor(x-1, y-1);
	}
}

void GTerm::device_attrib()
{
	SendBack("\033[?1;2c");
}

void GTerm::delete_char()
{
	int n, mx;
	n = param[0]; if (n<1) n = 1;
	mx = width-cursor_x;
	if (n>=mx) {
		clear_area(cursor_x, cursor_y, width-1, cursor_y);
	} else {
		shift_text(cursor_y, cursor_x, width-1, -n);
	}
}

void GTerm::set_mode()  // h
{
	switch (param[0] + 1000*q_mode) {
		case 1007:	clear_mode_flag(NOEOLWRAP);	break;
		case 1001:	set_mode_flag(CURSORAPPMODE);	break;
		case 1006:	set_mode_flag(CURSORRELATIVE);	break;
		case 4:		set_mode_flag(INSERT);		break;
		case 1003:	RequestSizeChange(132, height);	break;
		case 20:	set_mode_flag(NEWLINE);		break;
		case 12:	clear_mode_flag(LOCALECHO);	break;
		case 1025:
			clear_mode_flag(CURSORINVISIBLE);
			move_cursor(cursor_x, cursor_y);
			break;
	}
}

void GTerm::clear_mode()  // l
{
	switch (param[0] + 1000*q_mode) {
		case 1007:	set_mode_flag(NOEOLWRAP);	break;
		case 1001:	clear_mode_flag(CURSORAPPMODE);	break;
		case 1006:	clear_mode_flag(CURSORRELATIVE); break;
		case 4:		clear_mode_flag(INSERT);	break;
		case 1003:	RequestSizeChange(80, height);	break;
		case 20:	clear_mode_flag(NEWLINE);	break;
		case 1002:	current_state = vt52_normal_state; break;
		case 12:	set_mode_flag(LOCALECHO);	break;
		case 1025:
			set_mode_flag(CURSORINVISIBLE);	break;
			move_cursor(cursor_x, cursor_y);
			break;
	}
}

void GTerm::request_param()
{
	char str[40];
	sprintf(str, "\033[%d;1;1;120;120;1;0x", param[0]+2);
	SendBack(str);
}

void GTerm::set_margins()
{
	int t, b;

	t = param[0];
	if (t<1) t = 1;
	b = param[1];
	if (b<1) b = height;
	if (b>height) b = height;

	if (pending_scroll) update_changes();

	scroll_top = t-1;
	scroll_bot = b-1;
	if (cursor_y < scroll_top) move_cursor(cursor_x, scroll_top);
	if (cursor_y > scroll_bot) move_cursor(cursor_x, scroll_bot);
}

void GTerm::delete_line()
{
	int n, mx;
	n = param[0]; if (n<1) n = 1;
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
	if (param[0] == 5) {
		SendBack("\033[0n");
	} else if (param[0] == 6) {
		sprintf(str, "\033[%d;%dR", cursor_y+1, cursor_x+1);
		SendBack(str);
	}
}

void GTerm::erase_display()
{
	switch (param[0]) {
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
	switch (param[0]) {
	case 0:
		clear_area(cursor_x, cursor_y, width, cursor_y);
		break;
	case 1:
		clear_area(0, cursor_y, cursor_x, cursor_y);
		break;
	case 2:
		clear_area(0, cursor_y, width, cursor_y);
		break;
	}
}

void GTerm::insert_line()
{
	int n, mx;
	n = param[0]; if (n<1) n = 1;
	mx = scroll_bot-cursor_y+1;
	if (n>=mx) {
		clear_area(0, cursor_y, width-1, scroll_bot);
	} else {
		scroll_region(cursor_y, scroll_bot, -n);
	}
}

void GTerm::set_colors() // imm: note - affects more than just colours...
{
	int n;

	if (nparam == 0 && param[0] == 0) {
		attributes = 0;
		fg_color = 7;
		bg_color = 0;
		return;
	}

	attributes = 0; 	// imm... linux console seems to leave underline on. This "fixes" that...
	for (n = 0; n <= nparam; n++) {
		if (param[n]/10 == 4) {
			bg_color = int_clamp(param[n]%10, 0, 7);
		} else if (param[n]/10 == 3) {
			fg_color = int_clamp(param[n]%10, 0, 7);
		} else {
			switch (param[n]) {
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
	if (param[0] == 3) {
		memset(tab_stops, 0, sizeof(tab_stops));
	} else if (param[0] == 0) {
		tab_stops[cursor_x] = false;
	}
}

void GTerm::insert_char()
{
	int n, mx;
	n = param[0]; if (n<1) n = 1;
	mx = width-cursor_x;
	if (n>=mx) {
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

void GTerm::erase_char()
{
	// number of characters to erase
	const int n = int_clamp(param[0], 1, width-cursor_x);
	clear_area(cursor_x, cursor_y, cursor_x+n-1, cursor_y);
}

void GTerm::vt52_cursory()
{
	const int y = int_clamp(*input_data-32, 0, height-1);
	param[0] = y;
}

void GTerm::vt52_cursorx()
{
	const int x = int_clamp(*input_data-32, 0, width-1);
	const int y = param[0];
	move_cursor(x, y);
}

void GTerm::vt52_ident()
{
	SendBack("\033/Z");
}

/* End of File */
