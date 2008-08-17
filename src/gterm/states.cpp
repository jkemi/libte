/* GRG: Added a lot of GTerm:: prefixes to correctly form pointers
 *   to functions in all the tables. Example: &cr -> &GTerm::cr
 */

// Copyright Timothy Miller, 1999

#include "gterm.hpp"

// these are documented here:
// http://www.xfree86.org/current/ctlseqs.html

// state machine transition tables
StateOption GTerm::normal_state[] = {
    { 13, &GTerm::cr,		normal_state },
    { 10, &GTerm::lf,		normal_state },
    { 12, &GTerm::ff,		normal_state },
    { 9,  &GTerm::tab,		normal_state },
    { 8,  &GTerm::bs,		normal_state },
    { 7,  &GTerm::bell,		normal_state },
	{ 27, 0,				esc_state },
    { -1, &GTerm::normal_input,	normal_state} };

StateOption GTerm::esc_state[] = {
    { '[', &GTerm::clear_param,			bracket_state },
    { '>', &GTerm::keypad_numeric,		normal_state },
    { '=', &GTerm::keypad_application,	normal_state },
    { '7', &GTerm::save_cursor,			normal_state },
    { '8', &GTerm::restore_cursor,		normal_state },
    { 'H', &GTerm::set_tab,				normal_state },
    { 'D', &GTerm::index_down,			normal_state },
    { 'M', &GTerm::index_up,			normal_state },
    { 'E', &GTerm::next_line,			normal_state },
    { 'c', &GTerm::reset,				normal_state },
	{ '(', 0,							cset_shiftin_state },
	{ ')', 0,							cset_shiftout_state },
	{ '#', 0,							hash_state },
    { 13, &GTerm::cr,					esc_state },    // standard VT100 wants
    { 10, &GTerm::lf,					esc_state },    // cursor controls in
    { 12, &GTerm::ff,					esc_state },    // the middle of ESC
    { 9,  &GTerm::tab,					esc_state },    // sequences
    { 8,  &GTerm::bs,					esc_state },
    { 7,  &GTerm::bell,					esc_state },
	{ -1, 0,							normal_state} };

// Should put cursor control characters in these groups as well.
// Maybe later.

StateOption GTerm::cset_shiftin_state[] = {
	{ 'A', 0,		normal_state },	// should set UK characters
	{ '0', 0,		normal_state },	// should set Business Gfx
	{ -1, 0,		normal_state },	// default to ASCII
	};

StateOption GTerm::cset_shiftout_state[] = {
	{ 'A', 0,		normal_state },	// should set UK characters
	{ '0', 0,		normal_state },	// should set Business Gfx
	{ -1, 0,		normal_state },	// default to ASCII
	};

StateOption GTerm::hash_state[] = {
    { '8', &GTerm::screen_align,   normal_state },
	{ -1, 0, normal_state} };

StateOption GTerm::bracket_state[] = {
    { '?', &GTerm::set_q_mode,		bracket_state },
    { '"', &GTerm::set_quote_mode,	bracket_state },
    { '0', &GTerm::param_digit,		bracket_state },
    { '1', &GTerm::param_digit,		bracket_state },
    { '2', &GTerm::param_digit,		bracket_state },
    { '3', &GTerm::param_digit,		bracket_state },
    { '4', &GTerm::param_digit,		bracket_state },
    { '5', &GTerm::param_digit,		bracket_state },
    { '6', &GTerm::param_digit,		bracket_state },
    { '7', &GTerm::param_digit,		bracket_state },
    { '8', &GTerm::param_digit,		bracket_state },
    { '9', &GTerm::param_digit,		bracket_state },
    { ';', &GTerm::next_param,		bracket_state },
    { 'D', &GTerm::cursor_left,		normal_state },
    { 'B', &GTerm::cursor_down,		normal_state },
    { 'C', &GTerm::cursor_right,	normal_state },
    { 'A', &GTerm::cursor_up,		normal_state },
    { 'G', &GTerm::column_position,	normal_state },
    { 'H', &GTerm::cursor_position,	normal_state },
    { 'f', &GTerm::cursor_position,	normal_state },
    { 'c', &GTerm::device_attrib,	normal_state },
    { 'd', &GTerm::line_position,	normal_state },
    { 'P', &GTerm::delete_char,		normal_state },
    { 'h', &GTerm::set_mode,		normal_state },
    { 'l', &GTerm::clear_mode,		normal_state },
    { 's', &GTerm::save_cursor,		normal_state },
    { 'u', &GTerm::restore_cursor,	normal_state },
    { 'x', &GTerm::request_param,	normal_state },
    { 'r', &GTerm::set_margins,		normal_state },
    { 'M', &GTerm::delete_line,		normal_state },
    { 'n', &GTerm::status_report,	normal_state },
    { 'J', &GTerm::erase_display,	normal_state },
    { 'K', &GTerm::erase_line,		normal_state },
    { 'L', &GTerm::insert_line,		normal_state },
    { 'm', &GTerm::char_attrs,		normal_state },
    { 'g', &GTerm::clear_tab,		normal_state },
    { '@', &GTerm::insert_char,		normal_state },
    { 'X', &GTerm::erase_char,		normal_state },
	{ 'p', 0,						normal_state },	// something to do with levels

    { 13, &GTerm::cr,	bracket_state },// standard VT100 wants
    { 10, &GTerm::lf,	bracket_state },// cursor controls in
    { 12, &GTerm::ff,	bracket_state },// the middle of ESC
    { 9,  &GTerm::tab,	bracket_state },// sequences
    { 8,  &GTerm::bs,	bracket_state },
    { 7,  &GTerm::bell,	bracket_state },
	{ -1, 0, normal_state} };

