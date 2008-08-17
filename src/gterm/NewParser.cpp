/*
 * NewParser.cpp
 *
 *  Created on: Aug 17, 2008
 *      Author: jakob
 */

#include <stdio.h>

#include "gterm.hpp"

#include "actions.hpp"

static void _vt_debug(const char* label, unsigned char final, vtparse_t* vt) {
	printf("%s", label);
	printf("final: 0x%02x", final);
	if (final >= 32) {
		printf(" ('%c')", final);
	}
	printf(" intermediate: 0x%02x, 0x%02x params:", vt->intermediate_chars[0], vt->intermediate_chars[1]);
	for (int i = 0; i < vt->num_params; i++) {
		printf(" 0x%02x", vt->params[i]);
	}
	printf (" (%d)\n", vt->num_params);
}


static void _vt_print(GTerm* gt, unsigned char c) {
	int32_t cp = c;
	gt->input(&cp, 1);
}

// Single-character functions
static void _vt_execute(GTerm* gt, unsigned char c) {
	switch (c) {
	case '\a':	ac_bell(gt);	break;	// BEL	bell (CTRL-G)
	case '\b':	ac_bs(gt);	break;	// BS	backspace (CTRL-H)
	case '\r':	ac_cr(gt);	break;	// CR	carriage return (CTRL-M)
	case 5:					break;	// ENQ	terminal status (CTRL-E)
	case '\f':	ac_ff(gt);	break;	// FF	form feed (CTRL-L)
	case '\n':	ac_lf(gt);	break;	// LF	line feed (CTRL-J)
	case 14:				break;	// SO	shift out (CTRL-N)
	case ' ':	_vt_print(gt, ' ');			break;	// SP	space
	case '\t':	ac_tab(gt);	break;	// TAG	horizontal tab (CTRL-I)
	case '\v':	ac_lf(gt);	break;	// VT	vertical tab (CTRL-K)
	case 15:				break;	// SI	shift in (CTRL-O)
	}
}

// Controls beginning with ESC
static void _vt_esc_dispatch(GTerm* gt, unsigned char c) {

	switch (gt->parser.intermediate_chars[0]) {
	case 0:
		break;

	case ' ':
	case '#':
	case '%':
	case '(':
	case ')':
	case '*':
	case '+':
//		_vt_debug("ignoring intermediate esc dispatch: ", c, &gt->parser);
		return;
	default:
		_vt_debug("unexpected intermediate: ", c, &gt->parser);
	}

	switch (c) {
/*
	// VT100 and up
	case '0':	// DEC Special Character and Line Drawing Set
	case 'A':	// United Kingdom (UK)
	case 'B':	// United States (USASCII)

	// VT220 and up
	case '4':	// Dutch
	case 'C':	// Finnish
	case '5':	// Finnish
	case 'R':	// French
	case 'Q':	// French Canadian
	case 'K':	// German
	case 'Y':	// Italian
	case 'E':	// Norwegian/Danish
	case '6':	// Norwegian/Danish
	case 'Z':	// Spanish
	case 'H':	// Swedish
	case '7':	// Swedish
	case '=':	// Swiss
		printf("character set request (charset: '%c', request: '%c')\n", c, gt->parser.intermediate_chars[0]);
		break;
*/

	case 'H':	ac_set_tab(gt);	break;		// Tab Set ( HTS is 0x88)
	case '=':	ac_keypad_application(gt);	break;	// Application Keypad (DECPAM)
	case '>':	ac_keypad_normal(gt);	break;		// Normal Keypad (DECPNM)
	case 'c':	ac_reset(gt); break;					// Full Reset (RIS)
	default:
		_vt_debug("unknown esc dispatch: ", c, &gt->parser);
	}
}

static void _vt_csi_dispatch(GTerm* gt, unsigned char c) {
	switch (c) {
	case 'A':	ac_cursor_up(gt);		break;	// Cursor Up P s Times (default = 1) (CUU)
	case 'C':	ac_cursor_right(gt);	break;	// Cursor Forward P s Times (default = 1) (CUF)
	case 'G':	ac_column_position(gt); break;	// Cursor Character Absolute [column] (default = [row,1]) (CHA)
	case 'H':	ac_cursor_position(gt); break;	// Cursor Position [row;column] (default = [1,1]) (CUP)
	case 'J':	ac_erase_display(gt);	break;	// Erase in Display (ED)
	case 'K':	ac_erase_line(gt);		break;	// Erase in Line (EL)
	case 'P':	ac_delete_char(gt);		break;	// Delete P s Character(s) (default = 1) (DCH)
	case 'X':	ac_erase_char(gt);		break;	// Erase P s Character(s) (default = 1) (ECH)
	case 'd':	ac_line_position(gt);	break; 	// Line Position Absolute [row] (default = [1,column]) (VPA)
	case 'h':	ac_set_mode(gt);		break;	// Set Mode (SM)
	case 'l':	ac_clear_mode(gt);		break;	// Reset Mode (RM)
	case 'm':	ac_char_attrs(gt);		break;	// Character Attributes (SGR)
	case 'p':	ac_reset(gt);			break;	// Soft terminal reset (DECSTR)
	case 'r':	ac_set_margins(gt);		break;	// Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)
	case 's':	ac_save_cursor(gt);		break;	// Save cursor (ANSI.SYS)
	default:
		_vt_debug("unknown csi dispatch: ", c, &gt->parser);
		//printf ("unknown csi dispatch: 0x%02x\n", c);
	}
}

static void _vt_parser_cb(struct vtparse* parser, vtparse_action_t action, unsigned char c) {
	GTerm* gt = (GTerm*)parser->user_data;

	switch (action) {
	case VTPARSE_ACTION_PRINT:
		_vt_print(gt, c);
		break;
	case VTPARSE_ACTION_EXECUTE:
		_vt_execute(gt, c);
		break;

	// Escape functions
	case VTPARSE_ACTION_ESC_DISPATCH:
		_vt_esc_dispatch(gt, c);
		break;

	// Control functions
	case VTPARSE_ACTION_CSI_DISPATCH:
		_vt_csi_dispatch(gt, c);
		break;

	// Protocol hook support
	case VTPARSE_ACTION_HOOK:
		printf("ignoring all hooks (request 0x%02x)\n", c);
		break;
	case VTPARSE_ACTION_PUT:
	case VTPARSE_ACTION_UNHOOK:

	// Operating system control
	case VTPARSE_ACTION_OSC_START:
		_vt_debug("osc start (ignored): ", c, parser);
		break;
	case VTPARSE_ACTION_OSC_PUT:
		_vt_debug("osc put (ignored): ", c, parser);
		break;
	case VTPARSE_ACTION_OSC_END:
		_vt_debug("osc end (ignored): ", c, parser);
		break;
	default:
		break;
	}

//	if (!gt->is_mode_set(gt->DEFERUPDATE) || gt->pending_scroll > gt->scroll_bot-gt->scroll_top) {
		gt->update_changes();
	//}
}


void parser_init (GTerm* gt) {
	vtparse_init(&gt->parser, &_vt_parser_cb);
	gt->parser.user_data = gt;
}
