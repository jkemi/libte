/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include "charsets.h"

// 0 - US ASCII
const te_chartable_entry_t chartable_us[] = {
	{'\0', '\0'},
};

// A - UK ('#' -> '£')
const te_chartable_entry_t chartable_uk[] = {
	{'#', 0x00a3},	// Pound Sign
	{'\0', '\0'},
};

// 0 - DEC Special Character and Line Drawing Set
const te_chartable_entry_t chartable_special[] = {
	{'`', 0x25c6},  // Diamond
	{'a', 0x2592},  // Checkerboard
	{'b', 0x2409},  // HT symbol
	{'c', 0x240c},  // FF symbol
	{'d', 0x240d},  // CR symbol
	{'e', 0x240a},  // LF symbol
	{'f', 0x00b0},  // Degree
	{'g', 0x00b1},  // Plus/minus
	{'h', 0x2424},  // NL symbol
	{'i', 0x240b},  // VT symbol
	{'j', 0x2518},  // Downright corner
	{'k', 0x2510},  // Upright corner
	{'l', 0x250c},  // Upleft corner
	{'m', 0x2514},  // Downleft corner
	{'n', 0x253c},  // Cross
	{'o', 0x23ba},  // Scan line 1/9
	{'p', 0x23bb},  // Scan line 3/9
	{'q', 0x2500},  // Horizontal line (also scan line 5/9)
	{'r', 0x23bc},  // Scan line 7/9
	{'s', 0x23bd},  // Scan line 9/9
	{'t', 0x251c},  // Left t
	{'u', 0x2524},  // Right t
	{'v', 0x2534},  // Bottom t
	{'w', 0x252c},  // Top t
	{'x', 0x2502},  // Vertical line
	{'y', 0x2264},  // <=
	{'z', 0x2265},  // >=
	{'{', 0x03c0},  // PI
	{'|', 0x2260},  // Not equal
	{'}', 0x00a3},  // Pound currency sign
	{'~', 0x00b7},  // Bullet
	{'\0', '\0'},
};
