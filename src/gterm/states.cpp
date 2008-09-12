// Copyright Timothy Miller, 1999

#include <stddef.h>
#include "Parser.h"
#include "actions.hpp"

#include "states.h"

extern const StateOption* const state_normal;
extern const StateOption* const state_esc;
extern const StateOption* const state_csi;
extern const StateOption* const state_osc;
extern const StateOption* const state_ignore_to_st;
extern const StateOption* const state_dcs;
extern const StateOption* const state_cset_shiftin;
extern const StateOption* const state_cset_shiftout;
extern const StateOption* const state_hash;

// these are documented here:
// http://www.xfree86.org/current/ctlseqs.html

// state machine transition tables
static const StateOption _state_normal[] = {
    { '\r', &ac_cr,		state_normal },	// CR
    { '\n', &ac_lf,		state_normal },	// LF
    { '\f', &ac_lf,		state_normal },	// FF (xterm spec says same as LF)
    { '\v', &ac_lf,		state_normal },	// VT (xterm spec says same as LF)
    { '\t', &ac_tab,	state_normal }, // HT
    { '\b', &ac_bs,		state_normal }, // BS
    { '\a', &ac_bell,	state_normal }, // BEL
	{ 27, NULL,			state_esc },

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

//const StateOption state_esc[] = {
static const StateOption _state_esc[] = {
    { '[', &_parser_clear_param,	state_csi },
    { ']', &_parser_osc_start,		state_osc },
    { '>', &ac_keypad_normal,		state_normal },
    { '=', &ac_keypad_application,	state_normal },
    { '7', &ac_save_cursor,			state_normal },
    { '8', &ac_restore_cursor,		state_normal },
    { 'H', &ac_set_tab,				state_normal },
    { 'D', &ac_index_down,			state_normal },	// Index (IND)
    { 'M', &ac_index_up,			state_normal }, // Reverse Index (RI)
    { 'E', &ac_next_line,			state_normal },
    { 'c', &ac_reset,				state_normal },
	{ '(', NULL,					state_cset_shiftin },
	{ ')', NULL,					state_cset_shiftout },
	{ '#', NULL,					state_hash },

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

// Should put cursor control characters in these groups as well.
// Maybe later.

static const StateOption _state_cset_shiftin[] = {
	{ 'A',	NULL,	state_normal },	// should set UK characters
	{ '0',	NULL,	state_normal },	// should set Business Gfx
	{ -1,	NULL,	state_normal },	// default to ASCII
};

static const StateOption _state_cset_shiftout[] = {
	{ 'A',	NULL,	state_normal },	// should set UK characters
	{ '0',	NULL,	state_normal },	// should set Business Gfx
	{ -1,	NULL,	state_normal },	// default to ASCII
};

static const StateOption _state_hash[] = {
    { '8',	&ac_screen_align,   state_normal },
	{ -1,	NULL,				state_normal}
};

static const StateOption _state_csi[] = {
    { '?', &_parser_set_intermediate,	state_csi },
    { '"', &_parser_set_intermediate,	state_csi },
    { '!', &_parser_set_intermediate,	state_csi },
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
    { '@', &ac_insert_char,		state_normal },
    { 'A', &ac_cursor_up,		state_normal },
    { 'B', &ac_cursor_down,		state_normal },
    { 'C', &ac_cursor_right,	state_normal },
    { 'D', &ac_cursor_left,		state_normal },
    { 'G', &ac_column_position,	state_normal },
    { 'H', &ac_cursor_position,	state_normal },
    { 'J', &ac_erase_display,	state_normal },
    { 'K', &ac_erase_line,		state_normal },
    { 'L', &ac_insert_line,		state_normal },
    { 'M', &ac_delete_line,		state_normal },
    { 'P', &ac_delete_char,		state_normal },
    { 'S', &ac_scroll_up,		state_normal },	// Scroll Up (SU)
    { 'T', &ac_scroll_down,		state_normal },	// Scroll Down (SD)
    { 'X', &ac_erase_char,		state_normal },
    { 'c', &ac_device_attrib,	state_normal },	// Send Device Attributes (Primary DA)
    { 'd', &ac_line_position,	state_normal },
    { 'f', &ac_cursor_position,	state_normal },
    { 'g', &ac_clear_tab,		state_normal },
    { 'h', &ac_set_mode,		state_normal },
    { 'l', &ac_clear_mode,		state_normal },
    { 'm', &ac_char_attrs,		state_normal },
    { 'n', &ac_status_report,	state_normal },
	{ 'p', &ac_set_conformance,	state_normal },	// Set conformance level (DECSCL) and Soft terminal reset (DECSTR)
    { 'r', &ac_set_margins,		state_normal },
    { 's', &ac_save_cursor,		state_normal },
    { 'u', &ac_restore_cursor,	state_normal },
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

static const StateOption _state_osc[] = {
	{ '\a',	&_parser_osc_end,		state_normal },
	{ 0x9c,	&_parser_osc_end,		state_normal },	// 8-bit ST
	{ 27, 	&_parser_osc_end,		state_esc	 },
	{ -1, 	&_parser_osc_put,		state_osc	}
};

// APC and PM are ignored (as per xterm spec.)
static const StateOption _state_ignore_to_st[] = {
	{ 0x9c,	NULL,		state_normal },	// 8-bit ST
	{ 27, 	NULL,		state_esc	 },
	{ -1, 	NULL,		state_ignore_to_st	}
};

// Device-Control functions
static const StateOption _state_dcs[] = {
	{ 0x9c,	&_parser_dcs_end,		state_normal },	// 8-bit ST
	{ 27, 	&_parser_dcs_end,		state_esc	 },
	{ -1, 	&_parser_dcs_put,		state_dcs	}
};


const StateOption* const state_normal =			_state_normal;
const StateOption* const state_esc =			_state_esc;
const StateOption* const state_csi =			_state_csi;
const StateOption* const state_osc =			_state_osc;
const StateOption* const state_ignore_to_st =	_state_ignore_to_st;
const StateOption* const state_dcs =			_state_dcs;
const StateOption* const state_cset_shiftin =	_state_cset_shiftin;
const StateOption* const state_cset_shiftout =	_state_cset_shiftout;
const StateOption* const state_hash =			_state_hash;
