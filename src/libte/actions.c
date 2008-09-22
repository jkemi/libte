/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 * Some parts are copyright (c) 1999 by Timothy Miller.
 */


//
// Methods called from state machine
//

#include "internal.h"

#include "buffer.h"
#include "misc.h"
#include "history.h"
#include "viewport.h"
#include "parser.h"

#include "actions.h"

void ac_cr(TE* te)
{
	be_move_cursor(te, 0, te->cursor_y);
}

// Line-Feed (same as Vertical-Tab and Form-Feed)
void ac_lf(TE* te)
{
	ac_index_down(te);

	if (be_is_mode_flag(te, MODE_NEWLINE)) {
		ac_cr(te);
	}
}

// Horizontal Tabulation Set (HTS)
void ac_tab(TE* te)
{
	int x = -1;

	// find nearest set tab-stop
	for (int i = te->cursor_x+1; i < te->width; i++) {
		if (te->tab_stops[i]) {
			x = i;
			break;
		}
	}

	// set tab-stop not found, use 8 spaces
	if (x < 0) {
		x = (te->cursor_x+7) & ~0x7;
	}

	if (x < te->width) {
		be_move_cursor(te, x, te->cursor_y);
	} else {
		// Tabs _never_ causes newline
		be_move_cursor(te, te->width-1, te->cursor_y);
	}
}

void ac_bs(TE* te)
{
	if (te->cursor_x > 0) {
		be_move_cursor(te, te->cursor_x-1, te->cursor_y);
	}
	if (be_is_mode_set(te, MODE_DESTRUCTBS)) {
		be_clear_area(te, te->cursor_x, te->cursor_y, 1, 1);
	}
}

void ac_bell(TE* te)
{
	fe_bell(te);
}

void ac_keypad_normal(TE* te) {
	be_clear_mode_flag(te, MODE_KEYAPP);
}

void ac_keypad_application(TE* te) {
	be_set_mode_flag(te, MODE_KEYAPP);
}

// Save Cursor (DECSC)
void ac_save_cursor(TE* te)
{
	te->stored.attributes = te->attributes;
	te->stored.cursor_x = te->cursor_x;
	te->stored.cursor_y = te->cursor_y;
	te->stored.autowrap = be_is_mode_flag(te, MODE_AUTOWRAP);
}

// Restore Cursor (DECRC)
void ac_restore_cursor(TE* te)
{
	te->attributes = te->stored.attributes;
	if (te->stored.autowrap) {
		be_set_mode_flag(te, MODE_AUTOWRAP);
	} else {
		be_clear_mode_flag(te, MODE_AUTOWRAP);
	}
	be_move_cursor(te, te->stored.cursor_x, te->stored.cursor_y);
}

void ac_set_tab(TE* te)
{
	te->tab_stops[te->cursor_x] = true;
}

// Index (IND)
// IND moves the cursor down one line in the same column.
// If the cursor is at the bottom margin, then the screen performs a scroll-up.
void ac_index_down(TE* te)
{
	if (te->cursor_y == te->scroll_bot) {
		be_scroll_region(te, te->scroll_top, te->scroll_bot, 1);
	} else {
		be_move_cursor(te, te->cursor_x, te->cursor_y+1);
	}
}

// Next line (NEL)
// Moves cursor to first position on next line. If cursor is at bottom margin,
// then screen performs a scroll-up.
void ac_next_line(TE* te)
{
	ac_index_down(te);
	ac_cr(te);
}

void ac_index_up(TE* te)
{
	if (te->cursor_y == te->scroll_top) {
		be_scroll_region(te, te->scroll_top, te->scroll_bot, -1);
	} else {
		be_move_cursor(te, te->cursor_x, te->cursor_y-1);
	}
}

// Soft Terminal Reset DECSTR
//
//   Full Reset (RIS) is here implemented in the same way as DECSTR
//   due to the emulated nature.
void ac_reset(TE* te)
{
	te->bg_color = SYMBOL_BG_DEFAULT;
	te->fg_color = SYMBOL_FG_DEFAULT;
	te->scroll_top = 0;
	te->scroll_bot = te->height-1;
	memset(te->tab_stops, 0, sizeof(bool)*te->width);

	be_set_mode(te, MODE_AUTOWRAP);

	te->attributes = 0;

	be_clear_area(te, 0, 0, te->width, te->height);
	be_move_cursor(te, 0, 0);

	history_clear(&te->history);
	viewport_set(te, 0);
	viewport_taint_all(te);
}

// Cursor Backward P s Times (default = 1) (CUB)
void ac_cursor_left(TE* te)
{
	int n, x;
	n = int_max(1, parser_get_param(te->parser,0,1) );

	x = int_max(0, te->cursor_x-n);
	be_move_cursor(te, x, te->cursor_y);
}

// Cursor Forward P s Times (default = 1) (CUF)
void ac_cursor_right(TE* te)
{
	int n, x;
	n = int_max(1, parser_get_param(te->parser,0,1) );

	x = int_min(te->width-1, te->cursor_x+n);
	be_move_cursor(te, x, te->cursor_y);
}

// Cursor Up P s Times (default = 1) (CUU)
void ac_cursor_up(TE* te)
{
	int n, y;
	n = int_max(1, parser_get_param(te->parser,0,1) );

	if (be_is_mode_set(te, MODE_ORIGIN)) {
		y = int_max(te->scroll_top, te->cursor_y-n);
	} else {
		y = int_max(0, te->cursor_y-n);
	}
	be_move_cursor(te, te->cursor_x, y);
}

// Cursor Down P s Times (default = 1) (CUD)
void ac_cursor_down(TE* te)
{
	int n, y;
	n = int_max(1, parser_get_param(te->parser,0,1) );

	if (be_is_mode_set(te, MODE_ORIGIN)) {
		y = int_min(te->scroll_bot, te->cursor_y+n);
	} else {
		y = int_min(te->height-1, te->cursor_y+n);
	}
	be_move_cursor(te, te->cursor_x, y);
}

// Cursor Position (CUP)
void ac_cursor_position(TE* te)
{
	int y, x;

	if (be_is_mode_set(te, MODE_ORIGIN)) {
		y = int_clamp(parser_get_param(te->parser, 0, 1)+te->scroll_top, te->scroll_top, te->scroll_bot+1);
	} else {
		y = int_clamp(parser_get_param(te->parser, 0, 1), 1, te->height);
	}
	x = int_clamp(parser_get_param(te->parser, 1, 1), 1, te->width);

	be_move_cursor(te, x-1, y-1);
}

// Cursor Character Absolute [column] (default = [row,1]) (CHA)
void ac_column_position(TE* te)
{
	int	x = int_clamp(parser_get_param(te->parser, 0, 1), 1, te->width);

	be_move_cursor(te, x-1, te->cursor_y);
}

// Line Position Absolute [row] (default = [1,column]) (VPA)
void ac_line_position(TE* te)
{
	int	y = int_clamp(parser_get_param(te->parser, 0, 1), 1, te->height);

	be_move_cursor(te, te->cursor_x, y-1);
}

// 	Send Device Attributes (Primary DA)
void ac_device_attrib(TE* te)
{
	fe_send_back_char(te, "\033[?1;2c");
}

// Delete P s Character(s) (default = 1) (DCH)
void ac_delete_char(TE* te)
{
	int n, mx;
	n = int_max(1, parser_get_param(te->parser,0,1) );

	mx = te->width - te->cursor_x;
	if (n >= mx) {
		be_clear_area(te, te->cursor_x, te->cursor_y, te->width-te->cursor_x, 1);
	} else {
		BufferRow* row = buffer_get_row(&te->buffer, te->cursor_y);
		bufrow_remove(row,te->cursor_x,n);
		viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
	}
}

// Set Mode (SM)
void ac_set_mode(TE* te)  // h
{

	const int p = parser_get_param(te->parser,0,-1);

	if (parser_get_intermediate(te->parser) == '?') {
		// DEC Private Mode Set (DECSET)
		// Lots of these are missing

		switch (p) {
		case 1:	be_set_mode_flag(te, MODE_CURSORAPP);	break;	// Normal Cursor Keys (DECCKM)
//		case 2:													// Designate VT52 mode (DECANM).
		case 3:													// 132 Column Mode (DECCOLM)
			fe_request_resize(te, 132, te->height);
			be_clear_area(te, 0, 0, te->width, te->height);
			break;
		case 6: be_set_mode_flag(te, MODE_ORIGIN);		break;	// Origin mode (DECOM)
		case 7:	be_set_mode_flag(te, MODE_AUTOWRAP);	break;	// Wraparound Mode (DECAWM)
		case 25:												// Hide Cursor (DECTCEM)
			be_clear_mode_flag(te, MODE_CURSORINVISIBLE);
			be_move_cursor(te, te->cursor_x, te->cursor_y);
			break;
		default:
			DEBUGF("unhandled private set mode (DECSET) mode: %d\n", p);
			break;
		}
	} else {
		// Set Mode (SM)

		switch (p) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		be_set_mode_flag(te, MODE_INSERT);	  	break;	// Insert Mode (IRM)
		case 12:	be_clear_mode_flag(te, MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	be_set_mode_flag(te, MODE_NEWLINE);		break;	// Automatic Newline (LNM)
		default:
			DEBUGF("unhandled set mode (SM) mode: %d\n", p);
			break;
		}
	}
}

// Reset Mode (RM)
void ac_clear_mode(TE* te)  // l
{
	if (parser_get_intermediate(te->parser) == '?') {
		// DEC Private Mode Reset (DECRST)
		// Lots of these are missing

		switch (parser_get_param(te->parser,0,-1)) {
		case 1:	be_clear_mode_flag(te, MODE_CURSORAPP);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:	current_state = vt52_normal_state; break;			// Designate VT52 mode (DECANM).
		case 3:
			fe_request_resize(te, 80, te->height);				// 132 Column Mode (DECCOLM)
			be_clear_area(te, 0, 0, te->width, te->height);
			break;
		case 6: be_clear_mode_flag(te, MODE_ORIGIN);			break;	// Origin mode (DECOM)
		case 7:	be_clear_mode_flag(te, MODE_AUTOWRAP);			break;	// Wraparound Mode (DECAWM)
		case 25:														// Hide Cursor (DECTCEM)
			be_set_mode_flag(te, MODE_CURSORINVISIBLE);	break;
			be_move_cursor(te, te->cursor_x, te->cursor_y);
			break;
		}
	} else {
		// Reset Mode (RM)

		switch (parser_get_param(te->parser,0,-1)) {
//		case 2:														// Keyboard Action Mode (AM)
		case 4:		be_clear_mode_flag(te, MODE_INSERT);	break;	// Insert Mode (IRM)
		case 12:	be_set_mode_flag(te, MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	be_clear_mode_flag(te, MODE_NEWLINE);	break;	// Automatic Newline (LNM)
		}
	}
}

// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
void ac_set_conformance	(TE* te) {
	if (parser_get_intermediate(te->parser) == '!') {
		ac_reset(te);
	} else {
		// TODO: not implemented yet.
	}
}

void ac_request_param(TE* te)
{
	// TODO: what is this function for???



	char str[40];
	sprintf(str, "\033[%d;1;1;120;120;1;0x", parser_get_param(te->parser, 0, 0)+2);

	fe_send_back_char(te, str);
}

// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
void ac_set_margins(TE* te)
{
	int t, b;

	t = int_clamp(parser_get_param(te->parser,0,1), 1, te->height-1);
	b = int_clamp(parser_get_param(te->parser,1,te->height), t+1, te->height);

	DEBUGF("scrolling region set to: %d,%d\n", t,b);

	te->scroll_top = t-1;
	te->scroll_bot = b-1;

	if (be_is_mode_flag(te, MODE_ORIGIN)) {
		be_move_cursor(te, te->scroll_top, 0);
	} else {
		be_move_cursor(te, 0, 0);
	}
}

// Delete P s Line(s) (default = 1) (DL)
void ac_delete_line(TE* te)
{
	int n, mx;
	n = int_max(parser_get_param(te->parser, 0, 1), 1);
	mx = te->scroll_bot-te->cursor_y+1;
	if (n>=mx) {
		be_clear_area(te, 0, te->cursor_y, te->width, te->scroll_bot-te->cursor_y);
	} else {
		be_scroll_region(te, te->cursor_y, te->scroll_bot, n);
	}
}

// Device Status Report (DSR)
void ac_status_report(TE* te)
{
	char str[20];
	switch (parser_get_param(te->parser, 0, -1)) {
	case 5:
		fe_send_back_char(te, "\033[0n");
		break;
	case 6:
		sprintf(str, "\033[%d;%dR", te->cursor_y+1, te->cursor_x+1);
		fe_send_back_char(te, str);
		break;
	}
}

// Erase in Display (ED)
void ac_erase_display(TE* te)
{
	switch ( parser_get_param(te->parser, 0, 0) ) {
	case 0:	// Erase Below (default)
		be_clear_area(te, te->cursor_x, te->cursor_y, te->width-te->cursor_x, 1);
		if (te->cursor_y < te->height-1) {
			be_clear_area(te, 0, te->cursor_y+1, te->width, te->height-te->cursor_y-1);
		}
		break;
	case 1: // Erase Above
		be_clear_area(te, 0, te->cursor_y, te->cursor_x+1, 1);
		if (te->cursor_y > 0) {
			be_clear_area(te, 0, 0, te->width, te->cursor_y);
		}
		break;
	case 2: // Erase All
		be_clear_area(te, 0, 0, te->width, te->height);
		break;
	case 3:	// Erase Saved Lines (xterm)
	default:
		DEBUGF("unhandled parameter in erase in display\n");
		break;
	}
}

// Erase in Line (EL)
void ac_erase_line(TE* te)
{
	switch ( parser_get_param(te->parser, 0, 0) ) {
	case 0:	// Erase to Right (default)
		be_clear_area(te, te->cursor_x, te->cursor_y, te->width-te->cursor_x, 1);
		break;
	case 1:	// Erase to Left
		be_clear_area(te, 0, te->cursor_y, te->cursor_x+1, 1);
		break;
	case 2: // Erase All
		be_clear_area(te, 0, te->cursor_y, te->width, 1);
		break;
	}
}

// Insert P s Line(s) (default = 1) (IL)
void ac_insert_line(TE* te)
{
	int n, mx;
	n = int_max(1, parser_get_param(te->parser, 0, 1) );
	mx = te->scroll_bot-te->cursor_y+1;
	if (n >= mx) {
		be_clear_area(te, 0, te->cursor_y, te->width, te->scroll_bot-te->cursor_y);
	} else {
		be_scroll_region(te, te->cursor_y, te->scroll_bot, -n);
	}
}

// Character Attributes (SGR)
void ac_char_attrs(TE* te)
{
	int n;

	const int nparams = parser_get_nparams(te->parser);

	if (nparams == 0){
		te->attributes = 0;
		te->fg_color = SYMBOL_FG_DEFAULT;
		te->bg_color = SYMBOL_BG_DEFAULT;
		return;
	}

	const int* params = parser_get_params(te->parser);
	for (n = 0; n < nparams; n++) {
		const int p = params[n];
		if (p/10 == 4) {
			if (p%10 == 9) {
				te->bg_color = SYMBOL_BG_DEFAULT;
			} else {
				te->bg_color = int_clamp(p%10, 0, 7);
			}
		} else if (p/10 == 3) {
			if (p%10 == 9) {
				te->fg_color = SYMBOL_FG_DEFAULT;
			} else {
				te->fg_color = int_clamp(p%10, 0, 7);
			}
		} else {
			switch (p) {
			case 0:
				te->attributes = 0;
				te->fg_color = SYMBOL_FG_DEFAULT;
				te->bg_color = SYMBOL_BG_DEFAULT;
				break;
			case 1: te->attributes |= SYMBOL_BOLD; 	break;
			case 4: te->attributes |= SYMBOL_UNDERLINE; break;
			case 5:	te->attributes |= SYMBOL_BLINK; 	break;
			case 7:	te->attributes |= SYMBOL_INVERSE; 	break;
			case 22:  te->attributes &= (~SYMBOL_BOLD);	break;
			case 24:  te->attributes &= (~SYMBOL_UNDERLINE);	break;
			case 25:  te->attributes &= (~SYMBOL_BLINK);	break;
			case 27:  te->attributes &= (~SYMBOL_INVERSE);	break;

			case 100:
				te->fg_color = SYMBOL_FG_DEFAULT;
				te->bg_color = SYMBOL_BG_DEFAULT;
				break;

			default:
				DEBUGF("recieved unhandled character attribute: %d\n", p);
			}
		}
	}
}

// Tab Clear (TBC)
void ac_clear_tab(TE* te)
{
	switch ( parser_get_param(te->parser,0,0) ) {
	case 3:
		memset(te->tab_stops, 0, sizeof(te->tab_stops));
		break;
	case 0:
		te->tab_stops[te->cursor_x] = false;
	}
}

// Insert P s (Blank) Character(s) (default = 1) (ICH)
void ac_insert_char(TE* te)
{
	int n, mx;
	n = int_max(1, parser_get_param(te->parser,0,1) );
	mx = te->width-te->cursor_x;
	if (n >= mx) {
		be_clear_area(te, te->cursor_x, te->cursor_y, te->width-te->cursor_x, 1);
	} else {
		BufferRow* row = buffer_get_row(&te->buffer, te->cursor_y);

		// TODO: remove this buffer
		symbol_t buf[n];
		for (int i = 0; i < n; i++) {
			buf[i] = ' ';
		}
		bufrow_insert(row, te->cursor_x, buf, n);
		viewport_taint(te, te->cursor_y, te->cursor_x, te->width-te->cursor_x);
	}
}

// DEC Screen Alignment Test (DECALN)
void ac_screen_align(TE* te)
{
	const symbol_t style = symbol_make_style(7,0,0);
	const symbol_t sym = 'E' | style;

	// TODO: fixup
	symbol_t syms[te->width];
	for (int x = 0; x < te->width; x++) {
		syms[x] = sym;
	}

	for (int y=0; y<te->height; y++) {
		BufferRow* row = buffer_get_row(&te->buffer, y);
		viewport_taint(te, y, 0, te->width);
		bufrow_replace(row, 0, syms, te->width);
	}
}

// Erase P s Character(s) (default = 1) (ECH)
void ac_erase_char(TE* te)
{
	// number of characters to erase
	const int n = int_clamp(parser_get_param(te->parser,0,1), 1, te->width-te->cursor_x);
	be_clear_area(te, te->cursor_x, te->cursor_y, n, 1);
}

// Scroll up Ps lines (default = 1) (SU)
// (Pan Down)
// Ps is the number of lines to move the user window down in page memory.
// Ps new lines appear at the bottom of the display. Ps old lines disappear at the top
// of the display. You cannot pan past the bottom margin of the current page.
void ac_scroll_up (TE* te) {
	const int n = int_clamp(parser_get_param(te->parser,0,1), 1, te->scroll_bot-te->scroll_top);

	DEBUGF("scroll up by %d\n", n);

	be_scroll_region(te, te->scroll_top, te->scroll_bot, n);
}

// Scroll up Ps lines (default = 1) (SD)
// (Pan Up)
void ac_scroll_down (TE* te) {
	const int n = int_clamp(parser_get_param(te->parser,0,1), 1, te->scroll_bot-te->scroll_top);

	DEBUGF("scroll down by %d\n", n);

	be_scroll_region(te, te->scroll_top, te->scroll_bot, -n);
}
