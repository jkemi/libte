/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 *
 * Some parts are copyright (c) 1999 by Timothy Miller.
 */


#include <stddef.h>
#include "actions.h"

#include "parser_internal.h"
#include "parser_states.h"


// these are documented here:
// http://www.xfree86.org/current/ctlseqs.html

static const StateOption state_esc[];
static const StateOption state_csi[];
static const StateOption state_osc[];
static const StateOption state_ignore_to_st[];
static const StateOption state_dcs[];
static const StateOption state_cset_shiftin[];
static const StateOption state_cset_shiftout[];
static const StateOption state_hash[];

// state machine transition tables
const StateOption state_normal[] = {
    { '\a', &ac_bell,	state_normal }, // BEL	- 7
    { '\b', &ac_bs,		state_normal }, // BS	- 8
    { '\t', &ac_tab,	state_normal }, // HT	- 9
    { '\n', &ac_lf,		state_normal },	// LF	- 10
    { '\v', &ac_lf,		state_normal },	// VT 	- 11	(xterm spec says same as LF)
    { '\f', &ac_lf,		state_normal },	// FF	- 12	(xterm spec says same as LF)
    { '\r', &ac_cr,		state_normal },	// CR	- 13
    { 14, &ac_ls_g1,			state_normal }, // SO	- 016 SHIFT OUT ( invoke G0 charset, as designated by SCS )
    { 15, &ac_ls_g0,			state_normal }, // SI	- 017 SHIFT IN ( invoke G1 charset, as selected by <ESC>( )
    { 24, NULL,			state_normal }, // CAN  - 030 if sent during a control-seq, it's ignored and displays error character
    { 26, NULL,			state_normal }, // SUB  - 032 same as CAN
	{ 27, NULL,			state_esc },	// ESC	- 033

	// 8-bit codes below
	{ 0x84, &ac_index_down, state_normal }, 	// Index (IND)
	{ 0x85, &ac_next_line, state_normal }, 		// Next Line (NE)
	{ 0x88, &ac_set_tab, state_normal }, 		// Tab Set (HTS)
	{ 0x8d, &ac_index_up, state_normal }, 		// Reverse Index (RI)
	{ 0x8e, NULL, state_normal },				// Single Shift Select of G2 Character Set (SS2) - not yet implemented
	{ 0x8f, NULL, state_normal }, 				// Single Shift Select of G3 Character Set (SS3) - not yet implemented
	{ 0x90, &_parser_dcs_start, state_dcs },	// Device Control String (DCS)
	{ 0x96, NULL, state_normal }, 				// Start of Guarded Area (SPA) - not yet implemented
	{ 0x97, NULL, state_ignore_to_st },			// End of Guarded Area (EPA) - not yet implemented
	{ 0x98, NULL, state_normal },				// Start of string (SOS) - not yet implemented
	{ 0x9a, NULL, state_normal }, 				// Return Terminal ID (DECID) - not yet implemented
	{ 0x9b, &_parser_clear_param, state_csi },	// Control Sequence Introducer (CSI)
	{ 0x9c, NULL, state_normal }, 				// String Terminator (ST) - ignored
	{ 0x9d, &_parser_osc_start, state_osc },	// Operating System Command (OSC)
	{ 0x9e, NULL, state_ignore_to_st },			// Privacy Message (PM)
	{ 0x9f, NULL, state_ignore_to_st },			// Application Program Command (APC)


    { -1, &_parser_normal_input,	state_normal}
};

static const StateOption state_esc[] = {
    { '[', &_parser_clear_param,	state_csi },
    { ']', &_parser_osc_start,		state_osc },
    { '>', &ac_keypad_normal,		state_normal },
    { '=', &ac_keypad_application,	state_normal },
    { '7', &ac_save_cursor,			state_normal },
    { '8', &ac_restore_cursor,		state_normal },
    { 'H', &ac_set_tab,				state_normal },
    { 'D', &ac_index_down,			state_normal },	// Index (IND)
    { 'M', &ac_index_up,			state_normal }, // Reverse Index (RI)
    { 'E', &ac_next_line,			state_normal }, // Next Line (NEL)
    { 'c', &ac_reset,				state_normal },
	{ '(', NULL,					state_cset_shiftin },
	{ ')', NULL,					state_cset_shiftout },
	{ '#', NULL,					state_hash },
//	{ '*', NULL,					state_asterisk },		vt220
//	{ '+', NULL,					state_plus },			vt220

	{ '^', NULL, state_ignore_to_st },		// Privacy Message (PM)
	{ '_', NULL, state_ignore_to_st },		// Application Program Command (APC)
	{ 'P', &_parser_dcs_start, state_dcs },	// Device Control String (DCS)

	// standard VT100 wants cursor controls in the middle of ESC sequences
    { '\r', &ac_cr,		state_esc },	// CR
    { '\n', &ac_lf,		state_esc },	// LF
    { '\f', &ac_lf,		state_esc },	// FF (xterm spec says same as LF)
    { '\v', &ac_lf,		state_esc },	// VT (xterm spec says same as LF)
    { '\t', &ac_tab,	state_esc },	// HT
    { '\b', &ac_bs,		state_esc },	// BS
    { '\a', &ac_bell,	state_esc },	// BEL

    { -1, &_parser_unknown_esc,		state_normal}
};

/* charset names:
	4 -> Dutch.
	C or 5 -> Finnish.
	R -> French.
	Q -> French Canadian.
	K -> German.
	Y -> Italian.
	E or 6 -> Norwegian/Danish.
	Z -> Spanish.
	H or 7 -> Swedish.
	= -> Swiss.
*/


// set SHIFT-IN charset (g0)
static const StateOption state_cset_shiftin[] = {
	{ 'A',	&ac_g0_set_uk,	state_normal },	// UK
	{ 'B',	&ac_g0_set_us,	state_normal },	// US-ASCII
	{ '0',	&ac_g0_set_sg,	state_normal },	// DEC Special Character and Line Drawing Set
//	{ '1',	NULL,			state_normal },	// Alternate Character ROM Standard Character Set
//	{ '2',	NULL,			state_normal },	// Alternate Character ROM Special Graphics

	{ -1,	&ac_g0_set_us,			state_normal },	// default to ASCII
};

// set SHIFT-OUT charset (g1)
static const StateOption state_cset_shiftout[] = {
	{ 'A',	&ac_g1_set_uk,	state_normal },	// UK
	{ 'B',	&ac_g1_set_us,	state_normal },	// US-ASCII
	{ '0',	&ac_g1_set_sg,	state_normal },	// DEC Special Character and Line Drawing Set
//	{ '1',	NULL,	state_normal },	// Alternate Character ROM Standard Character Set
//	{ '2',	NULL,	state_normal },	// Alternate Character ROM Special Graphics

	{ -1,	&ac_g1_set_us,	state_normal },	// default to ASCII
};

static const StateOption state_hash[] = {
    { '8',	&ac_screen_align,   state_normal },	// (DECALN)
	{ -1,	NULL,				state_normal}
};

// set g2 vt220
// ESC # <charset>
/*static const StateOption state_asterisk[] = {
	{ 'A',	&ac_g2_set_uk,	state_normal },	// UK
	{ 'B',	&ac_g2_set_us,	state_normal },	// US-ASCII
	{ '0',	&ac_g2_set_sg,	state_normal },	// DEC Special Character and Line Drawing Set

	{ -1,	NULL,			state_normal}
};
*/

// set g3 vt220
// ESC + <charset>
/*
static const StateOption state_plus[] = {
	{ 'A',	&ac_g2_set_uk,	state_normal },	// UK
	{ 'B',	&ac_g2_set_us,	state_normal },	// US-ASCII
	{ '0',	&ac_g2_set_sg,	state_normal },	// DEC Special Character and Line Drawing Set
	{ -1,	NULL,			state_normal}
};
*/

static const StateOption state_csi[] = {
    { '?', &_parser_set_intermediate,	state_csi },
    { '"', &_parser_set_intermediate,	state_csi },
    { '!', &_parser_set_intermediate,	state_csi },
    { '>', &_parser_set_intermediate,	state_csi },
    { '0', &_parser_param_digit,	state_csi },
    { '1', &_parser_param_digit,	state_csi },
    { '2', &_parser_param_digit,	state_csi },
    { '3', &_parser_param_digit,	state_csi },
    { '4', &_parser_param_digit,	state_csi },
    { '5', &_parser_param_digit,	state_csi },
    { '6', &_parser_param_digit,	state_csi },
    { '7', &_parser_param_digit,	state_csi },
    { '8', &_parser_param_digit,	state_csi },
    { '9', &_parser_param_digit,	state_csi },
    { ';', &_parser_next_param,		state_csi },
    { '@', &ac_insert_char,		state_normal },	// ICH
    { 'A', &ac_cursor_up,		state_normal }, // CUU
    { 'B', &ac_cursor_down,		state_normal }, // CUD
    { 'C', &ac_cursor_right,	state_normal }, // CUF
    { 'D', &ac_cursor_left,		state_normal }, // CUB
    { 'G', &ac_column_position,	state_normal },	// CHA
    { '`', &ac_column_position,	state_normal },	// HPA
    { 'H', &ac_cursor_position,	state_normal },	// CUP
    { 'J', &ac_erase_display,	state_normal }, // ED
    { 'K', &ac_erase_line,		state_normal },	// EL
    { 'L', &ac_insert_line,		state_normal }, // IL
    { 'M', &ac_delete_line,		state_normal }, // DL
    { 'P', &ac_delete_char,		state_normal }, // DCH
    { 'S', &ac_scroll_up,		state_normal },	// Scroll Up (SU)
    { 'T', &ac_scroll_down,		state_normal },	// Scroll Down (SD)
    { 'X', &ac_erase_char,		state_normal }, // ECH
    { 'c', &ac_device_attrib,	state_normal },	// Send Device Attributes (Primary DA)
    { 'd', &ac_line_position,	state_normal }, // VPA
    { 'f', &ac_cursor_position,	state_normal },	// HVP
    { 'g', &ac_clear_tab,		state_normal }, // TBC
    { 'h', &ac_set_mode,		state_normal }, // SM
    { 'l', &ac_clear_mode,		state_normal }, // RM
    { 'm', &ac_char_attrs,		state_normal }, // SGR
    { 'n', &ac_status_report,	state_normal }, // DSR
	{ 'p', &ac_set_conformance,	state_normal },	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
    { 'r', &ac_set_margins,		state_normal }, // DECSTBM
    { 's', &ac_save_cursor,		state_normal }, // DECSC
    { 'u', &ac_restore_cursor,	state_normal }, // DECRC
    { 'x', &ac_request_param,	state_normal },

	// standard VT100 wants cursor controls in the middle of ESC sequences
    { '\r', &ac_cr,		state_csi },	// CR
    { '\n', &ac_lf,		state_csi },	// LF
    { '\f', &ac_lf,		state_csi },	// FF (xterm spec says same as LF)
    { '\v', &ac_lf,		state_csi },	// VT (xterm spec says same as LF)
    { '\t', &ac_tab,	state_csi },	// HT
    { '\b', &ac_bs,		state_csi },	// BS
    { '\a', &ac_bell,	state_csi },	// BEL

    { -1,   &_parser_unknown_csi,		state_normal	}
 };

static const StateOption state_osc[] = {
	{ '\a',	&_parser_osc_end,		state_normal },
	{ 0x9c,	&_parser_osc_end,		state_normal },	// 8-bit ST
	{ 27, 	&_parser_osc_end,		state_esc	 },

	{ -1, 	&_parser_osc_put,		state_osc	}
};

// APC and PM are ignored (as per xterm spec.)
static const StateOption state_ignore_to_st[] = {
	{ 0x9c,	NULL,		state_normal },	// 8-bit ST
	{ 27, 	NULL,		state_esc	 },
	{ -1, 	NULL,		state_ignore_to_st	}
};

// Device-Control functions
static const StateOption state_dcs[] = {
	{ 0x9c,	&_parser_dcs_end,		state_normal },	// 8-bit ST
	{ 27, 	&_parser_dcs_end,		state_esc	 },
	{ -1, 	&_parser_dcs_put,		state_dcs	}
};
