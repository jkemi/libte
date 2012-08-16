/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef BUFFERSYMBOL_H_
#define BUFFERSYMBOL_H_

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

// additional data layout:
// bits     987654321
// fg-color       xxx
// bg-color    xxx
typedef uint_fast16_t symbol_data_t;
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

static inline symbol_data_t symbol_get_data(symbol_t sym) {
	return sym >> 22;
}

static inline int symbol_get_codepoint(symbol_t sym) {
	return (sym & 0x3fffff);
}

static inline symbol_t symbol_set_data(symbol_t sym, symbol_data_t data) {
	return (sym & 0x3fffff) | (data<<22);
}

static inline symbol_t symbol_get_style (symbol_t sym) {
	return (sym & ~0x3fffff);
}

static inline symbol_color_t symbol_get_fg(symbol_t sym) {
	return symbol_get_data(sym) & 0x7;
}

static inline symbol_color_t symbol_get_bg(symbol_t sym) {
	return (symbol_get_data(sym) & 0x38) >> 3;
}

static inline symbol_attributes_t symbol_get_attributes(symbol_t sym) {
	return (symbol_get_data(sym)&0x1c0) >> 6;
}

static inline symbol_t symbol_set_fg(symbol_t sym, symbol_color_t col) {
	return sym | ((symbol_t)col << 22);
}

static inline symbol_t symbol_set_bg(symbol_t sym, symbol_color_t col) {
	return sym | ((symbol_t)col << 25);
}

static inline symbol_t symbol_make_style(symbol_color_t fg, symbol_color_t bg, symbol_attributes_t attributes) {
	return ((symbol_t)bg)<<25 | ((symbol_t)fg)<<22 | ((symbol_t)attributes)<<28;
}

#endif
