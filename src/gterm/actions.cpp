// Copyright Timothy Miller, 1999

//
// Methods called from state machine
//

#include "gterm.hpp"

#include "Buffer.h"
#include "misc.h"

#include "actions.hpp"

static inline int _get_param(GTerm* gt, int index, int default_value) {
	if (gt->parser.num_params > index) {
		return gt->parser.params[index];
	} else {
		return default_value;
	}
}

void ac_cr(GTerm* gt)
{
	gt->move_cursor(0, gt->cursor_y);
}

// Line-Feed (same as Vertical-Tab and Form-Feed)
void ac_lf(GTerm* gt)
{
	ac_index_down(gt);

	if (gt->is_mode_flag(MODE_NEWLINE)) {
		ac_cr(gt);
	}
}

// Horizontal Tabulation Set (HTS)
void ac_tab(GTerm* gt)
{
	int x = -1;

	// find nearest set tab-stop
	for (int i = gt->cursor_x+1; i < gt->width; i++) {
		if (gt->tab_stops[i]) {
			x = i;
			break;
		}
	}

	// set tab-stop not found, use 8 spaces
	if (x < 0) {
		x = (gt->cursor_x+7) & ~0x7;
	}

	if (x < gt->width) {
		gt->move_cursor(x, gt->cursor_y);
	} else {
		// Tabs _never_ causes newline
		gt->move_cursor(gt->width-1, gt->cursor_y);
	}
}

void ac_bs(GTerm* gt)
{
	if (gt->cursor_x > 0) {
		gt->move_cursor(gt->cursor_x-1, gt->cursor_y);
	}
	if (gt->is_mode_set(MODE_DESTRUCTBS)) {
		gt->clear_area(gt->cursor_x, gt->cursor_y, gt->cursor_x, gt->cursor_y);
	}
}

void ac_bell(GTerm* gt)
{
	gt->_fe->bell(gt->_fe_priv);
}

void ac_keypad_normal(GTerm* gt) {
	gt->clear_mode_flag(MODE_KEYAPP);
}

void ac_keypad_application(GTerm* gt) {
	gt->set_mode_flag(MODE_KEYAPP);
}

// Save Cursor (DECSC)
void ac_save_cursor(GTerm* gt)
{
	gt->stored.attributes = gt->attributes;
	gt->stored.cursor_x = gt->cursor_x;
	gt->stored.cursor_y = gt->cursor_y;
	gt->stored.autowrap = gt->is_mode_flag(MODE_AUTOWRAP);
}

// Restore Cursor (DECRC)
void ac_restore_cursor(GTerm* gt)
{
	gt->attributes = gt->stored.attributes;
	if (gt->stored.autowrap) {
		gt->set_mode_flag(MODE_AUTOWRAP);
	} else {
		gt->clear_mode_flag(MODE_AUTOWRAP);
	}
	gt->move_cursor(gt->stored.cursor_x, gt->stored.cursor_y);
}

void ac_set_tab(GTerm* gt)
{
	gt->tab_stops[gt->cursor_x] = true;
}

// Index (IND)
// IND moves the cursor down one line in the same column.
// If the cursor is at the bottom margin, then the screen performs a scroll-up.
void ac_index_down(GTerm* gt)
{
	if (gt->cursor_y == gt->scroll_bot) {
		gt->scroll_region(gt->scroll_top, gt->scroll_bot, 1);
	} else {
		gt->move_cursor(gt->cursor_x, gt->cursor_y+1);
	}
}

// Next line (NEL)
// Moves cursor to first position on next line. If cursor is at bottom margin,
// then screen performs a scroll-up.
void ac_next_line(GTerm* gt)
{
	ac_index_down(gt);
	ac_cr(gt);
}

void ac_index_up(GTerm* gt)
{
	if (gt->cursor_y == gt->scroll_top) {
		gt->scroll_region(gt->scroll_top, gt->scroll_bot, -1);
	} else {
		gt->move_cursor(gt->cursor_x, gt->cursor_y-1);
	}
}

// Soft Terminal Reset DECSTR
//
//   Full Reset (RIS) is here implemented in the same way as DECSTR
//   due to the emulated nature.
void ac_reset(GTerm* gt)
{
	gt->pending_scroll = 0;
	gt->bg_color = SYMBOL_BG_DEFAULT;
	gt->fg_color = SYMBOL_FG_DEFAULT;
	gt->scroll_top = 0;
	gt->scroll_bot = gt->height-1;
	memset(gt->tab_stops, 0, sizeof(bool)*gt->width);

	gt->set_mode(MODE_AUTOWRAP);

	gt->attributes = 0;

	gt->clear_area(0, 0, gt->width, gt->height-1);
	gt->move_cursor(0, 0);
}

// Cursor Backward P s Times (default = 1) (CUB)
void ac_cursor_left(GTerm* gt)
{
	int n, x;
	n = int_max(1, _get_param(gt,0,1) );

	x = int_max(0, gt->cursor_x-n);
	gt->move_cursor(x, gt->cursor_y);
}

// Cursor Forward P s Times (default = 1) (CUF)
void ac_cursor_right(GTerm* gt)
{
	int n, x;
	n = int_max(1, _get_param(gt,0,1) );

	x = int_min(gt->width-1, gt->cursor_x+n);
	gt->move_cursor(x, gt->cursor_y);
}

// Cursor Up P s Times (default = 1) (CUU)
void ac_cursor_up(GTerm* gt)
{
	int n, y;
	n = int_max(1, _get_param(gt,0,1) );

	if (gt->is_mode_set(MODE_ORIGIN)) {
		y = int_max(gt->scroll_top, gt->cursor_y-n);
	} else {
		y = int_max(0, gt->cursor_y-n);
	}
	gt->move_cursor(gt->cursor_x, y);
}

// Cursor Down P s Times (default = 1) (CUD)
void ac_cursor_down(GTerm* gt)
{
	int n, y;
	n = int_max(1, _get_param(gt,0,1) );

	if (gt->is_mode_set(MODE_ORIGIN)) {
		y = int_min(gt->scroll_bot, gt->cursor_y+n);
	} else {
		y = int_min(gt->height-1, gt->cursor_y+n);
	}
	gt->move_cursor(gt->cursor_x, y);
}

// Cursor Position (CUP)
void ac_cursor_position(GTerm* gt)
{
	int y, x;

	if (gt->is_mode_set(MODE_ORIGIN)) {
		y = int_clamp(_get_param(gt, 0, 1)+gt->scroll_top, gt->scroll_top, gt->scroll_bot+1);
	} else {
		y = int_clamp(_get_param(gt, 0, 1), 1, gt->height);
	}
	x = int_clamp(_get_param(gt, 1, 1), 1, gt->width);

	gt->move_cursor(x-1, y-1);
}

// Cursor Character Absolute [column] (default = [row,1]) (CHA)
void ac_column_position(GTerm* gt)
{
	int	x = int_clamp(_get_param(gt, 0, 1), 1, gt->width);

	gt->move_cursor(x-1, gt->cursor_y);
}

// Line Position Absolute [row] (default = [1,column]) (VPA)
void ac_line_position(GTerm* gt)
{
	int	y = int_clamp(_get_param(gt, 0, 1), 1, gt->height);

	gt->move_cursor(gt->cursor_x, y-1);
}


void ac_device_attrib(GTerm* gt)
{
	gt->fe_send_back("\033[?1;2c");
}

// Delete P s Character(s) (default = 1) (DCH)
void ac_delete_char(GTerm* gt)
{
	int n, mx;
	n = int_max(1, _get_param(gt,0,1) );

	mx = gt->width-gt->cursor_x;
	if (n >= mx) {
		gt->clear_area(gt->cursor_x, gt->cursor_y, gt->width-1, gt->cursor_y);
	} else {
		gt->shift_text(gt->cursor_y, gt->cursor_x, gt->width-1, -n);
	}
}

// Set Mode (SM)
void ac_set_mode(GTerm* gt)  // h
{

	const int p = _get_param(gt,0,-1);

	if (gt->parser.intermediate_chars[0] == '?') {
		// DEC Private Mode Set (DECSET)
		// Lots of these are missing

		switch (p) {
		case 1:	gt->set_mode_flag(MODE_CURSORAPP);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:											// Designate VT52 mode (DECANM).
		case 3:	gt->fe_request_resize(132, gt->height);	break;	// 132 Column Mode (DECCOLM)
		case 6: gt->set_mode_flag(MODE_ORIGIN);			break;	// Origin mode (DECOM)
		case 7:	gt->set_mode_flag(MODE_AUTOWRAP);		break;	// Wraparound Mode (DECAWM)
		case 25:										// Hide Cursor (DECTCEM)
			gt->clear_mode_flag(MODE_CURSORINVISIBLE);
			gt->move_cursor(gt->cursor_x, gt->cursor_y);
			break;
		default:
			printf ("unhandled private set mode (DECSET) mode: %d\n", p);
			break;
		}
	} else {
		// Set Mode (SM)

		switch (p) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		gt->set_mode_flag(MODE_INSERT);		  	break;	// Insert Mode (IRM)
		case 12:	gt->clear_mode_flag(MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	gt->set_mode_flag(MODE_NEWLINE);		break;	// Automatic Newline (LNM)
		default:
			printf ("unhandled set mode (SM) mode: %d\n", p);
			break;
		}
	}
}

// Reset Mode (RM)
void ac_clear_mode(GTerm* gt)  // l
{
	if (gt->parser.intermediate_chars[0] == '?') {
		// DEC Private Mode Reset (DECRST)
		// Lots of these are missing

		switch (_get_param(gt,0,-1)) {
		case 1:	gt->clear_mode_flag(MODE_CURSORAPP);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:	current_state = vt52_normal_state; break;	// Designate VT52 mode (DECANM).
		case 3:	gt->fe_request_resize(80, gt->height);		break;	// 132 Column Mode (DECCOLM)
		case 6: gt->clear_mode_flag(MODE_ORIGIN);			break;	// Origin mode (DECOM)
		case 7:	gt->clear_mode_flag(MODE_AUTOWRAP);			break;	// Wraparound Mode (DECAWM)
		case 25:											// Hide Cursor (DECTCEM)
			gt->set_mode_flag(MODE_CURSORINVISIBLE);	break;
			gt->move_cursor(gt->cursor_x, gt->cursor_y);
			break;
		}
	} else {
		// Reset Mode (RM)

		switch (_get_param(gt,0,-1)) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		gt->clear_mode_flag(MODE_INSERT);	break;	// Insert Mode (IRM)
		case 12:	gt->set_mode_flag(MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	gt->clear_mode_flag(MODE_NEWLINE);	break;	// Automatic Newline (LNM)
		}
	}
}

// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
void ac_set_conformance	(GTerm* gt) {
	if (gt->parser.intermediate_chars[0] == '!') {
		ac_reset(gt);
	} else {
		// TODO: not implemented yet.
	}
}

void ac_request_param(GTerm* gt)
{
	char str[40];
	sprintf(str, "\033[%d;1;1;120;120;1;0x", gt->parser.params[0]+2);

	gt->fe_send_back(str);
}

// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
void ac_set_margins(GTerm* gt)
{
	int t, b;

	t = int_clamp(_get_param(gt,0,1), 1, gt->height-1);
	b = int_clamp(_get_param(gt,1,gt->height), t+1, gt->height);

	printf("scrolling region set to: %d,%d\n", t,b);

	if (gt->pending_scroll) {
		gt->update_changes();
	}

	gt->scroll_top = t-1;
	gt->scroll_bot = b-1;

	if (gt->is_mode_flag(MODE_ORIGIN)) {
		gt->move_cursor(gt->scroll_top, 0);
	} else {
		gt->move_cursor(0, 0);
	}
}

// Delete P s Line(s) (default = 1) (DL)
void ac_delete_line(GTerm* gt)
{
	int n, mx;
	n = int_max(_get_param(gt, 0, 1), 1);
	mx = gt->scroll_bot-gt->cursor_y+1;
	if (n>=mx) {
		gt->clear_area(0, gt->cursor_y, gt->width-1, gt->scroll_bot);
	} else {
		gt->scroll_region(gt->cursor_y, gt->scroll_bot, n);
	}
}

// Device Status Report (DSR)
void ac_status_report(GTerm* gt)
{
	char str[20];
	switch (_get_param(gt, 0, -1)) {
	case 5:
		gt->fe_send_back("\033[0n");
		break;
	case 6:
		sprintf(str, "\033[%d;%dR", gt->cursor_y+1, gt->cursor_x+1);
		gt->fe_send_back(str);
		break;
	}
}

// Erase in Display (ED)
void ac_erase_display(GTerm* gt)
{
	switch ( _get_param(gt, 0, 0) ) {
	case 0:
		gt->clear_area(gt->cursor_x, gt->cursor_y, gt->width-1, gt->cursor_y);
		if (gt->cursor_y < gt->height-1) {
			gt->clear_area(0, gt->cursor_y+1, gt->width-1, gt->height-1);
		}
		break;
	case 1:
		gt->clear_area(0, gt->cursor_y, gt->cursor_x, gt->cursor_y);
		if (gt->cursor_y > 0) {
			gt->clear_area(0, 0, gt->width-1, gt->cursor_y-1);
		}
		break;
	case 2:
		gt->clear_area(0, 0, gt->width-1, gt->height-1);
		break;
	}
}

// Erase in Line (EL)
void ac_erase_line(GTerm* gt)
{
	switch ( _get_param(gt, 0, 0) ) {
	case 0:	// Erase to Right (default)
		gt->clear_area(gt->cursor_x, gt->cursor_y, gt->width, gt->cursor_y);
		break;
	case 1:	// Erase to Left
		gt->clear_area(0, gt->cursor_y, gt->cursor_x, gt->cursor_y);
		break;
	case 2: // Erase All
		gt->clear_area(0, gt->cursor_y, gt->width, gt->cursor_y);
		break;
	}
}

// Insert P s Line(s) (default = 1) (IL)
void ac_insert_line(GTerm* gt)
{
	int n, mx;
	n = int_max(1, _get_param(gt, 0, 1) );
	mx = gt->scroll_bot-gt->cursor_y+1;
	if (n >= mx) {
		gt->clear_area(0, gt->cursor_y, gt->width-1, gt->scroll_bot);
	} else {
		gt->scroll_region(gt->cursor_y, gt->scroll_bot, -n);
	}
}

// Character Attributes (SGR)
void ac_char_attrs(GTerm* gt)
{
	int n;

	if (gt->parser.num_params == 0){
		gt->attributes = 0;
		gt->fg_color = SYMBOL_FG_DEFAULT;
		gt->bg_color = SYMBOL_BG_DEFAULT;
		return;
	}

	for (n = 0; n < gt->parser.num_params; n++) {
		const int p = gt->parser.params[n];
		if (p/10 == 4) {
			if (p%10 == 9) {
				gt->bg_color = SYMBOL_BG_DEFAULT;
			} else {
				gt->bg_color = int_clamp(p%10, 0, 7);
			}
		} else if (p/10 == 3) {
			if (p%10 == 9) {
				gt->fg_color = SYMBOL_FG_DEFAULT;
			} else {
				gt->fg_color = int_clamp(p%10, 0, 7);
			}
		} else {
			switch (p) {
			case 0:
				gt->attributes = 0;
				gt->fg_color = SYMBOL_FG_DEFAULT;
				gt->bg_color = SYMBOL_BG_DEFAULT;
				break;
			case 1: gt->attributes |= SYMBOL_BOLD; 	break;
			case 4: gt->attributes |= SYMBOL_UNDERLINE; break;
			case 5:	gt->attributes |= SYMBOL_BLINK; 	break;
			case 7:	gt->attributes |= SYMBOL_INVERSE; 	break;
			case 22:  gt->attributes &= (~SYMBOL_BOLD);	break;
			case 24:  gt->attributes &= (~SYMBOL_UNDERLINE);	break;
			case 25:  gt->attributes &= (~SYMBOL_BLINK);	break;
			case 27:  gt->attributes &= (~SYMBOL_INVERSE);	break;

			case 100:
				gt->fg_color = SYMBOL_FG_DEFAULT;
				gt->bg_color = SYMBOL_BG_DEFAULT;
				break;

			default:
				printf("recieved unhandled character attribute: %d\n", p);
			}
		}
	}
}

// Tab Clear (TBC)
void ac_clear_tab(GTerm* gt)
{
	switch ( _get_param(gt,0,0) ) {
	case 3:
		memset(gt->tab_stops, 0, sizeof(gt->tab_stops));
		break;
	case 0:
		gt->tab_stops[gt->cursor_x] = false;
	}
}

// Insert P s (Blank) Character(s) (default = 1) (ICH)
void ac_insert_char(GTerm* gt)
{
	int n, mx;
	n = int_max(1, _get_param(gt,0,1) );
	mx = gt->width-gt->cursor_x;
	if (n >= mx) {
		gt->clear_area(gt->cursor_x, gt->cursor_y, gt->width-1, gt->cursor_y);
	} else {
		gt->shift_text(gt->cursor_y, gt->cursor_x, gt->width-1, n);
	}
}

// DEC Screen Alignment Test (DECALN)
void ac_screen_align(GTerm* gt)
{
	const symbol_t style = symbol_make_style(7,0,0);
	const symbol_t sym = 'E' | style;

	// TODO: fixup
	symbol_t syms[gt->width];
	for (int x = 0; x < gt->width; x++) {
		syms[x] = sym;
	}

	for (int y=0; y<gt->height; y++) {
		BufferRow* row = buffer_get_row(&gt->buffer, y);
		gt->changed_line(y, 0, gt->width-1);
		bufrow_replace(row, 0, syms, gt->width);
	}
}

// Erase P s Character(s) (default = 1) (ECH)
void ac_erase_char(GTerm* gt)
{
	// number of characters to erase
	const int n = int_clamp(_get_param(gt,0,1), 1, gt->width-gt->cursor_x);
	gt->clear_area(gt->cursor_x, gt->cursor_y, gt->cursor_x+n-1, gt->cursor_y);
}
