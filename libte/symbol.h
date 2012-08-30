/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef TE_SYMBOL_H_
#define TE_SYMBOL_H_

#include <stdint.h>

// TODO: SYMBOL_UNDERLINE will exceed 32 bits.. should be fixed somehow

// Contains one ISO10646 character with color information
// and flags.
//
// planes 4-13 (40000–DFFFF) are unused by unicode and might be
// used for something
//
// 9 bits (32 - 23) are used for additional information here
typedef uint32_t symbol_t;

// layout
// aaaa fffb bbcc cccc cccc cccc cccc cccc
//  a = attributes
//  f = foreground
//  b = background
//  c = codepoint
#define SYMBOL_ATTRIBUTES_SHIFTS	(28)
#define SYMBOL_ATTRIBUTES_MASK		(0xf<<SYMBOL_ATTRIBUTES_SHIFTS)
#define SYMBOL_FG_SHIFTS			(22)
#define SYMBOL_FG_MASK				(0x7<<SYMBOL_FG_SHIFTS)
#define SYMBOL_BG_SHIFTS			(25)
#define SYMBOL_BG_MASK				(0x7<<SYMBOL_BG_SHIFTS)
#define SYMBOL_CP_SHIFTS			(0)
#define SYMBOL_CP_MASK				(0x3fffff<<SYMBOL_CP_SHIFTS)



// additional data layout:
// bits     987654321
// fg-color       xxx
// bg-color    xxx
typedef uint_fast8_t symbol_color_t;


// these are the colors:
//   0 - Black
//   1 - Red
//   2 - Green
//   3 - Yellow
//   4 - Blue
//   5 - Magenta
//   6 - Cyan
//   7 - White

#define SYMBOL_BG_DEFAULT 0
#define SYMBOL_FG_DEFAULT 7

// TODO: These are the needed attributes
#define SYMBOL_BOLD			(1<<0)
#define SYMBOL_BLINK		(1<<1)
#define SYMBOL_INVERSE		(1<<2)
#define SYMBOL_UNDERLINE	(1<<3)


typedef uint_fast8_t symbol_attributes_t;

static inline int symbol_get_codepoint(symbol_t sym) {
	return (sym & SYMBOL_CP_MASK)>>SYMBOL_CP_SHIFTS;
}

static inline symbol_color_t symbol_get_fg(symbol_t sym) {
	return (sym & SYMBOL_FG_MASK) >> SYMBOL_FG_SHIFTS;
}

static inline symbol_color_t symbol_get_bg(symbol_t sym) {
	return (sym & SYMBOL_BG_MASK) >> SYMBOL_BG_SHIFTS;
}

static inline symbol_attributes_t symbol_get_attributes(symbol_t sym) {
	return (sym & SYMBOL_ATTRIBUTES_MASK) >> SYMBOL_ATTRIBUTES_SHIFTS;
}

static inline symbol_t symbol_set_attributes(symbol_t sym, symbol_attributes_t attrs) {
	return (sym & ~SYMBOL_ATTRIBUTES_MASK) | ( ((symbol_t)attrs << SYMBOL_ATTRIBUTES_SHIFTS) & SYMBOL_ATTRIBUTES_MASK );
}

static inline symbol_t symbol_set_fg(symbol_t sym, symbol_color_t col) {
	return (sym & ~SYMBOL_FG_MASK) | ((symbol_t)col << SYMBOL_FG_SHIFTS);
}

static inline symbol_t symbol_set_bg(symbol_t sym, symbol_color_t col) {
	return (sym & ~SYMBOL_BG_MASK) | ((symbol_t)col << SYMBOL_BG_SHIFTS);
}

static inline symbol_t symbol_make(symbol_color_t fg, symbol_color_t bg, symbol_attributes_t attributes, uint32_t cp) {
	return	((symbol_t)bg)<<SYMBOL_BG_SHIFTS |
			((symbol_t)fg)<<SYMBOL_FG_SHIFTS |
			((symbol_t)attributes)<<SYMBOL_ATTRIBUTES_SHIFTS |
			cp<<SYMBOL_CP_SHIFTS;
}

static inline symbol_t symbol_make_style(symbol_color_t fg, symbol_color_t bg, symbol_attributes_t attributes) {
	return ((symbol_t)bg)<<SYMBOL_BG_SHIFTS | ((symbol_t)fg)<<SYMBOL_FG_SHIFTS | ((symbol_t)attributes)<<SYMBOL_ATTRIBUTES_SHIFTS;
}

#endif	// TE_SYMBOL_H_
