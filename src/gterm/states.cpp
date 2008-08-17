/* GRG: Added a lot of GTerm:: prefixes to correctly form pointers
 *   to functions in all the tables. Example: &cr -> &GTerm::cr
 */

// Copyright Timothy Miller, 1999

#include "gterm.hpp"

#include "OldParser.hpp"
#include "actions.hpp"

extern const StateOption* state_normal;
extern const StateOption* state_esc;
extern const StateOption* state_bracket;
extern const StateOption* state_cset_shiftin;
extern const StateOption* state_cset_shiftout;
extern const StateOption* state_hash;

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
    { -1, &_parser_normal_input,	state_normal}
};

//const StateOption state_esc[] = {
static const StateOption _state_esc[] = {
    { '[', &_parser_clear_param,			state_bracket },
    { '>', &ac_keypad_normal,		state_normal },
    { '=', &ac_keypad_application,	state_normal },
    { '7', &ac_save_cursor,			state_normal },
    { '8', &ac_restore_cursor,		state_normal },
    { 'H', &ac_set_tab,				state_normal },
    { 'D', &ac_index_down,			state_normal },
    { 'M', &ac_index_up,			state_normal },
    { 'E', &ac_next_line,			state_normal },
    { 'c', &ac_reset,				state_normal },
	{ '(', NULL,					state_cset_shiftin },
	{ ')', NULL,					state_cset_shiftout },
	{ '#', NULL,					state_hash },

	// standard VT100 wants cursor controls in the middle of ESC sequences
    { '\r', &ac_cr,		state_esc },	// CR
    { '\n', &ac_lf,		state_esc },	// LF
    { '\f', &ac_lf,		state_esc },	// FF (xterm spec says same as LF)
    { '\v', &ac_lf,		state_esc },	// VT (xterm spec says same as LF)
    { '\t', &ac_tab,	state_esc },	// HT
    { '\b', &ac_bs,		state_esc },	// BS
    { '\a', &ac_bell,	state_esc },	// BEL

    { -1, NULL,						state_normal}
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

static const StateOption _state_bracket[] = {
    { '?', &_parser_set_q_mode,		state_bracket },
    { '"', &_parser_set_quote_mode,	state_bracket },
    { '0', &_parser_param_digit,	state_bracket },
    { '1', &_parser_param_digit,	state_bracket },
    { '2', &_parser_param_digit,	state_bracket },
    { '3', &_parser_param_digit,	state_bracket },
    { '4', &_parser_param_digit,	state_bracket },
    { '5', &_parser_param_digit,	state_bracket },
    { '6', &_parser_param_digit,	state_bracket },
    { '7', &_parser_param_digit,	state_bracket },
    { '8', &_parser_param_digit,	state_bracket },
    { '9', &_parser_param_digit,	state_bracket },
    { ';', &_parser_next_param,		state_bracket },
    { 'D', &ac_cursor_left,		state_normal },
    { 'B', &ac_cursor_down,		state_normal },
    { 'C', &ac_cursor_right,	state_normal },
    { 'A', &ac_cursor_up,		state_normal },
    { 'G', &ac_column_position,	state_normal },
    { 'H', &ac_cursor_position,	state_normal },
    { 'f', &ac_cursor_position,	state_normal },
    { 'c', &ac_device_attrib,	state_normal },
    { 'd', &ac_line_position,	state_normal },
    { 'P', &ac_delete_char,		state_normal },
    { 'h', &ac_set_mode,		state_normal },
    { 'l', &ac_clear_mode,		state_normal },
    { 's', &ac_save_cursor,		state_normal },
    { 'u', &ac_restore_cursor,	state_normal },
    { 'x', &ac_request_param,	state_normal },
    { 'r', &ac_set_margins,		state_normal },
    { 'M', &ac_delete_line,		state_normal },
    { 'n', &ac_status_report,	state_normal },
    { 'J', &ac_erase_display,	state_normal },
    { 'K', &ac_erase_line,		state_normal },
    { 'L', &ac_insert_line,		state_normal },
    { 'm', &ac_char_attrs,		state_normal },
    { 'g', &ac_clear_tab,		state_normal },
    { '@', &ac_insert_char,		state_normal },
    { 'X', &ac_erase_char,		state_normal },
	{ 'p', NULL,				state_normal },	// something to do with levels

	// standard VT100 wants cursor controls in the middle of ESC sequences
    { '\r', &ac_cr,		state_bracket },	// CR
    { '\n', &ac_lf,		state_bracket },	// LF
    { '\f', &ac_lf,		state_bracket },	// FF (xterm spec says same as LF)
    { '\v', &ac_lf,		state_bracket },	// VT (xterm spec says same as LF)
    { '\t', &ac_tab,	state_bracket },	// HT
    { '\b', &ac_bs,		state_bracket },	// BS
    { '\a', &ac_bell,	state_bracket },	// BEL

    { -1, NULL,		state_normal	}
 };

const StateOption* state_normal =			_state_normal;
const StateOption* state_esc =				_state_esc;
const StateOption* state_bracket =			_state_bracket;
const StateOption* state_cset_shiftin =		_state_cset_shiftin;
const StateOption* state_cset_shiftout =	_state_cset_shiftout;
const StateOption* state_hash =				_state_hash;
