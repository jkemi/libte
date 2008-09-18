// Copyright Timothy Miller, 1999

//
// Methods called from state machine
//

#include "gterm.h"

#include "buffer.h"
#include "misc.h"
#include "history.h"
#include "viewport.h"
#include "parser.h"

#include "actions.h"

void ac_cr(GTerm* gt)
{
	gt_move_cursor(gt, 0, gt->cursor_y);
}

// Line-Feed (same as Vertical-Tab and Form-Feed)
void ac_lf(GTerm* gt)
{
	ac_index_down(gt);

	if (gt_is_mode_flag(gt, MODE_NEWLINE)) {
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
		gt_move_cursor(gt, x, gt->cursor_y);
	} else {
		// Tabs _never_ causes newline
		gt_move_cursor(gt, gt->width-1, gt->cursor_y);
	}
}

void ac_bs(GTerm* gt)
{
	if (gt->cursor_x > 0) {
		gt_move_cursor(gt, gt->cursor_x-1, gt->cursor_y);
	}
	if (gt_is_mode_set(gt, MODE_DESTRUCTBS)) {
		gt_clear_area(gt, gt->cursor_x, gt->cursor_y, 1, 1);
	}
}

void ac_bell(GTerm* gt)
{
	gt->_fe->bell(gt->_fe_priv);
}

void ac_keypad_normal(GTerm* gt) {
	gt_clear_mode_flag(gt, MODE_KEYAPP);
}

void ac_keypad_application(GTerm* gt) {
	gt_set_mode_flag(gt, MODE_KEYAPP);
}

// Save Cursor (DECSC)
void ac_save_cursor(GTerm* gt)
{
	gt->stored.attributes = gt->attributes;
	gt->stored.cursor_x = gt->cursor_x;
	gt->stored.cursor_y = gt->cursor_y;
	gt->stored.autowrap = gt_is_mode_flag(gt, MODE_AUTOWRAP);
}

// Restore Cursor (DECRC)
void ac_restore_cursor(GTerm* gt)
{
	gt->attributes = gt->stored.attributes;
	if (gt->stored.autowrap) {
		gt_set_mode_flag(gt, MODE_AUTOWRAP);
	} else {
		gt_clear_mode_flag(gt, MODE_AUTOWRAP);
	}
	gt_move_cursor(gt, gt->stored.cursor_x, gt->stored.cursor_y);
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
		gt_scroll_region(gt, gt->scroll_top, gt->scroll_bot, 1);
	} else {
		gt_move_cursor(gt, gt->cursor_x, gt->cursor_y+1);
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
		gt_scroll_region(gt, gt->scroll_top, gt->scroll_bot, -1);
	} else {
		gt_move_cursor(gt, gt->cursor_x, gt->cursor_y-1);
	}
}

// Soft Terminal Reset DECSTR
//
//   Full Reset (RIS) is here implemented in the same way as DECSTR
//   due to the emulated nature.
void ac_reset(GTerm* gt)
{
	gt->bg_color = SYMBOL_BG_DEFAULT;
	gt->fg_color = SYMBOL_FG_DEFAULT;
	gt->scroll_top = 0;
	gt->scroll_bot = gt->height-1;
	memset(gt->tab_stops, 0, sizeof(bool)*gt->width);

	gt_set_mode(gt, MODE_AUTOWRAP);

	gt->attributes = 0;

	gt_clear_area(gt, 0, 0, gt->width, gt->height);
	gt_move_cursor(gt, 0, 0);

	history_clear(&gt->history);
	viewport_set(gt, 0);
	viewport_taint_all(gt);
}

// Cursor Backward P s Times (default = 1) (CUB)
void ac_cursor_left(GTerm* gt)
{
	int n, x;
	n = int_max(1, parser_get_param(gt->parser,0,1) );

	x = int_max(0, gt->cursor_x-n);
	gt_move_cursor(gt, x, gt->cursor_y);
}

// Cursor Forward P s Times (default = 1) (CUF)
void ac_cursor_right(GTerm* gt)
{
	int n, x;
	n = int_max(1, parser_get_param(gt->parser,0,1) );

	x = int_min(gt->width-1, gt->cursor_x+n);
	gt_move_cursor(gt, x, gt->cursor_y);
}

// Cursor Up P s Times (default = 1) (CUU)
void ac_cursor_up(GTerm* gt)
{
	int n, y;
	n = int_max(1, parser_get_param(gt->parser,0,1) );

	if (gt_is_mode_set(gt, MODE_ORIGIN)) {
		y = int_max(gt->scroll_top, gt->cursor_y-n);
	} else {
		y = int_max(0, gt->cursor_y-n);
	}
	gt_move_cursor(gt, gt->cursor_x, y);
}

// Cursor Down P s Times (default = 1) (CUD)
void ac_cursor_down(GTerm* gt)
{
	int n, y;
	n = int_max(1, parser_get_param(gt->parser,0,1) );

	if (gt_is_mode_set(gt, MODE_ORIGIN)) {
		y = int_min(gt->scroll_bot, gt->cursor_y+n);
	} else {
		y = int_min(gt->height-1, gt->cursor_y+n);
	}
	gt_move_cursor(gt, gt->cursor_x, y);
}

// Cursor Position (CUP)
void ac_cursor_position(GTerm* gt)
{
	int y, x;

	if (gt_is_mode_set(gt, MODE_ORIGIN)) {
		y = int_clamp(parser_get_param(gt->parser, 0, 1)+gt->scroll_top, gt->scroll_top, gt->scroll_bot+1);
	} else {
		y = int_clamp(parser_get_param(gt->parser, 0, 1), 1, gt->height);
	}
	x = int_clamp(parser_get_param(gt->parser, 1, 1), 1, gt->width);

	gt_move_cursor(gt, x-1, y-1);
}

// Cursor Character Absolute [column] (default = [row,1]) (CHA)
void ac_column_position(GTerm* gt)
{
	int	x = int_clamp(parser_get_param(gt->parser, 0, 1), 1, gt->width);

	gt_move_cursor(gt, x-1, gt->cursor_y);
}

// Line Position Absolute [row] (default = [1,column]) (VPA)
void ac_line_position(GTerm* gt)
{
	int	y = int_clamp(parser_get_param(gt->parser, 0, 1), 1, gt->height);

	gt_move_cursor(gt, gt->cursor_x, y-1);
}

// 	Send Device Attributes (Primary DA)
void ac_device_attrib(GTerm* gt)
{
	gt_fe_send_back(gt, "\033[?1;2c");
}

// Delete P s Character(s) (default = 1) (DCH)
void ac_delete_char(GTerm* gt)
{
	int n, mx;
	n = int_max(1, parser_get_param(gt->parser,0,1) );

	mx = gt->width - gt->cursor_x;
	if (n >= mx) {
		gt_clear_area(gt, gt->cursor_x, gt->cursor_y, gt->width-gt->cursor_x, 1);
	} else {
		BufferRow* row = buffer_get_row(&gt->buffer, gt->cursor_y);
		bufrow_remove(row,gt->cursor_x,n);
		viewport_taint(gt, gt->cursor_y, gt->cursor_x, gt->width-gt->cursor_x);
	}
}

// Set Mode (SM)
void ac_set_mode(GTerm* gt)  // h
{

	const int p = parser_get_param(gt->parser,0,-1);

	if (parser_get_intermediate(gt->parser) == '?') {
		// DEC Private Mode Set (DECSET)
		// Lots of these are missing

		switch (p) {
		case 1:	gt_set_mode_flag(gt, MODE_CURSORAPP);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:											// Designate VT52 mode (DECANM).
		case 3:	gt_fe_request_resize(gt, 132, gt->height);	break;	// 132 Column Mode (DECCOLM)
		case 6: gt_set_mode_flag(gt, MODE_ORIGIN);			break;	// Origin mode (DECOM)
		case 7:	gt_set_mode_flag(gt, MODE_AUTOWRAP);		break;	// Wraparound Mode (DECAWM)
		case 25:										// Hide Cursor (DECTCEM)
			gt_clear_mode_flag(gt, MODE_CURSORINVISIBLE);
			gt_move_cursor(gt, gt->cursor_x, gt->cursor_y);
			break;
		default:
			printf ("unhandled private set mode (DECSET) mode: %d\n", p);
			break;
		}
	} else {
		// Set Mode (SM)

		switch (p) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		gt_set_mode_flag(gt, MODE_INSERT);		  	break;	// Insert Mode (IRM)
		case 12:	gt_clear_mode_flag(gt, MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	gt_set_mode_flag(gt, MODE_NEWLINE);		break;	// Automatic Newline (LNM)
		default:
			printf ("unhandled set mode (SM) mode: %d\n", p);
			break;
		}
	}
}

// Reset Mode (RM)
void ac_clear_mode(GTerm* gt)  // l
{
	if (parser_get_intermediate(gt->parser) == '?') {
		// DEC Private Mode Reset (DECRST)
		// Lots of these are missing

		switch (parser_get_param(gt->parser,0,-1)) {
		case 1:	gt_clear_mode_flag(gt, MODE_CURSORAPP);		break;	// Normal Cursor Keys (DECCKM)
//		case 2:	current_state = vt52_normal_state; break;	// Designate VT52 mode (DECANM).
		case 3:	gt_fe_request_resize(gt, 80, gt->height);		break;	// 132 Column Mode (DECCOLM)
		case 6: gt_clear_mode_flag(gt, MODE_ORIGIN);			break;	// Origin mode (DECOM)
		case 7:	gt_clear_mode_flag(gt, MODE_AUTOWRAP);			break;	// Wraparound Mode (DECAWM)
		case 25:											// Hide Cursor (DECTCEM)
			gt_set_mode_flag(gt, MODE_CURSORINVISIBLE);	break;
			gt_move_cursor(gt, gt->cursor_x, gt->cursor_y);
			break;
		}
	} else {
		// Reset Mode (RM)

		switch (parser_get_param(gt->parser,0,-1)) {
//		case 2:											// Keyboard Action Mode (AM)
		case 4:		gt_clear_mode_flag(gt, MODE_INSERT);	break;	// Insert Mode (IRM)
		case 12:	gt_set_mode_flag(gt, MODE_LOCALECHO);	break;	// Send/receive (SRM)
		case 20:	gt_clear_mode_flag(gt, MODE_NEWLINE);	break;	// Automatic Newline (LNM)
		}
	}
}

// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
void ac_set_conformance	(GTerm* gt) {
	if (parser_get_intermediate(gt->parser) == '!') {
		ac_reset(gt);
	} else {
		// TODO: not implemented yet.
	}
}

void ac_request_param(GTerm* gt)
{
	// TODO: what is this function for???



	char str[40];
	sprintf(str, "\033[%d;1;1;120;120;1;0x", parser_get_param(gt->parser, 0, 0)+2);

	gt_fe_send_back(gt, str);
}

// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
void ac_set_margins(GTerm* gt)
{
	int t, b;

	t = int_clamp(parser_get_param(gt->parser,0,1), 1, gt->height-1);
	b = int_clamp(parser_get_param(gt->parser,1,gt->height), t+1, gt->height);

	printf("scrolling region set to: %d,%d\n", t,b);

	gt->scroll_top = t-1;
	gt->scroll_bot = b-1;

	if (gt_is_mode_flag(gt, MODE_ORIGIN)) {
		gt_move_cursor(gt, gt->scroll_top, 0);
	} else {
		gt_move_cursor(gt, 0, 0);
	}
}

// Delete P s Line(s) (default = 1) (DL)
void ac_delete_line(GTerm* gt)
{
	int n, mx;
	n = int_max(parser_get_param(gt->parser, 0, 1), 1);
	mx = gt->scroll_bot-gt->cursor_y+1;
	if (n>=mx) {
		gt_clear_area(gt, 0, gt->cursor_y, gt->width, gt->scroll_bot-gt->cursor_y);
	} else {
		gt_scroll_region(gt, gt->cursor_y, gt->scroll_bot, n);
	}
}

// Device Status Report (DSR)
void ac_status_report(GTerm* gt)
{
	char str[20];
	switch (parser_get_param(gt->parser, 0, -1)) {
	case 5:
		gt_fe_send_back(gt, "\033[0n");
		break;
	case 6:
		sprintf(str, "\033[%d;%dR", gt->cursor_y+1, gt->cursor_x+1);
		gt_fe_send_back(gt, str);
		break;
	}
}

// Erase in Display (ED)
void ac_erase_display(GTerm* gt)
{
	switch ( parser_get_param(gt->parser, 0, 0) ) {
	case 0:	// Erase Below (default)
		gt_clear_area(gt, gt->cursor_x, gt->cursor_y, gt->width-gt->cursor_x, 1);
		if (gt->cursor_y < gt->height-1) {
			gt_clear_area(gt, 0, gt->cursor_y+1, gt->width, gt->height-gt->cursor_y-1);
		}
		break;
	case 1: // Erase Above
		gt_clear_area(gt, 0, gt->cursor_y, gt->cursor_x+1, 1);
		if (gt->cursor_y > 0) {
			gt_clear_area(gt, 0, 0, gt->width, gt->cursor_y);
		}
		break;
	case 2: // Erase All
		gt_clear_area(gt, 0, 0, gt->width, gt->height);
		break;
	case 3:	// Erase Saved Lines (xterm)
	default:
		printf("unhandled parameter in erase in display\n");
		break;
	}
}

// Erase in Line (EL)
void ac_erase_line(GTerm* gt)
{
	switch ( parser_get_param(gt->parser, 0, 0) ) {
	case 0:	// Erase to Right (default)
		gt_clear_area(gt, gt->cursor_x, gt->cursor_y, gt->width-gt->cursor_x, 1);
		break;
	case 1:	// Erase to Left
		gt_clear_area(gt, 0, gt->cursor_y, gt->cursor_x+1, 1);
		break;
	case 2: // Erase All
		gt_clear_area(gt, 0, gt->cursor_y, gt->width, 1);
		break;
	}
}

// Insert P s Line(s) (default = 1) (IL)
void ac_insert_line(GTerm* gt)
{
	int n, mx;
	n = int_max(1, parser_get_param(gt->parser, 0, 1) );
	mx = gt->scroll_bot-gt->cursor_y+1;
	if (n >= mx) {
		gt_clear_area(gt, 0, gt->cursor_y, gt->width, gt->scroll_bot-gt->cursor_y);
	} else {
		gt_scroll_region(gt, gt->cursor_y, gt->scroll_bot, -n);
	}
}

// Character Attributes (SGR)
void ac_char_attrs(GTerm* gt)
{
	int n;

	const int nparams = parser_get_nparams(gt->parser);

	if (nparams == 0){
		gt->attributes = 0;
		gt->fg_color = SYMBOL_FG_DEFAULT;
		gt->bg_color = SYMBOL_BG_DEFAULT;
		return;
	}

	const int* params = parser_get_params(gt->parser);
	for (n = 0; n < nparams; n++) {
		const int p = params[n];
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
	switch ( parser_get_param(gt->parser,0,0) ) {
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
	n = int_max(1, parser_get_param(gt->parser,0,1) );
	mx = gt->width-gt->cursor_x;
	if (n >= mx) {
		gt_clear_area(gt, gt->cursor_x, gt->cursor_y, gt->width-gt->cursor_x, 1);
	} else {
		BufferRow* row = buffer_get_row(&gt->buffer, gt->cursor_y);

		// TODO: remove this buffer
		symbol_t buf[n];
		for (int i = 0; i < n; i++) {
			buf[i] = ' ';
		}
		bufrow_insert(row, gt->cursor_x, buf, n);
		viewport_taint(gt, gt->cursor_y, gt->cursor_x, gt->width-gt->cursor_x);
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
		viewport_taint(gt, y, 0, gt->width);
		bufrow_replace(row, 0, syms, gt->width);
	}
}

// Erase P s Character(s) (default = 1) (ECH)
void ac_erase_char(GTerm* gt)
{
	// number of characters to erase
	const int n = int_clamp(parser_get_param(gt->parser,0,1), 1, gt->width-gt->cursor_x);
	gt_clear_area(gt, gt->cursor_x, gt->cursor_y, n, 1);
}

// Scroll up Ps lines (default = 1) (SU)
// (Pan Down)
// Ps is the number of lines to move the user window down in page memory.
// Ps new lines appear at the bottom of the display. Ps old lines disappear at the top
// of the display. You cannot pan past the bottom margin of the current page.
void ac_scroll_up (GTerm* gt) {
	const int n = int_clamp(parser_get_param(gt->parser,0,1), 1, gt->scroll_bot-gt->scroll_top);

	printf("scroll up by %d\n", n);

	gt_scroll_region(gt, gt->scroll_top, gt->scroll_bot, n);
}

// Scroll up Ps lines (default = 1) (SD)
// (Pan Up)
void ac_scroll_down (GTerm* gt) {
	const int n = int_clamp(parser_get_param(gt->parser,0,1), 1, gt->scroll_bot-gt->scroll_top);

	printf("scroll down by %d\n", n);

	gt_scroll_region(gt, gt->scroll_top, gt->scroll_bot, -n);
}
