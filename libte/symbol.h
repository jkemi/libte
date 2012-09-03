/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef TE_SYMBOL_H_
#define TE_SYMBOL_H_

#include <stdint.h>

//#undef TE_EXT
#define TE_EXT

#ifndef TE_EXT

// Contains one ISO10646 character with color information and attributes.
// UNICODE 6.1 spans 0-10FFFF (21 bits) we use 20 (last bit is private use, plane 16)
//
// planes 3-13 (30000–DFFFF) are unused by unicode and might be
// used for something
//
// planes 15,16 (F0000-FFFFD, 100000-10FFFD) are "private-use" areas and will probably not be needed.
//
typedef uint32_t symbol_t;

#if 1

// 32-bit layout
// aaaa ffff bbbb cccc cccc cccc cccc cccc
//	- = unused
//  a = attributes
//  f = foreground
//  b = background
//  c = codepoint
#define SYMBOL_ATTRIBUTES_SHIFTS	(28)
#define SYMBOL_ATTRIBUTES_MASK		(0xf<<SYMBOL_ATTRIBUTES_SHIFTS)	// 4 bits
#define SYMBOL_FG_SHIFTS			(24)
#define SYMBOL_FG_MASK				(0xf<<SYMBOL_FG_SHIFTS)			// 4 bits
#define SYMBOL_BG_SHIFTS			(20)
#define SYMBOL_BG_MASK				(0xf<<SYMBOL_BG_SHIFTS)			// 4 bits
#define SYMBOL_CP_SHIFTS			(0)
#define SYMBOL_CP_MASK				(0xfffff<<SYMBOL_CP_SHIFTS)		// 20 bits
#define SYMBOL_COLOR_BITS			4

#else

// Alternate 32-bit layout (knowing that only non-western codepoints exists beyond 2fff)
// that will allow 7 bits of color information (128 colors)
// aaaa ffff fffb bbbb bbcc cccc cccc cccc
//  a = attributes
//  f = foreground
//  b = background
//  c = codepoint
#define SYMBOL_ATTRIBUTES_SHIFTS	(28)
#define SYMBOL_ATTRIBUTES_MASK		(0xf<<SYMBOL_ATTRIBUTES_SHIFTS)	// 4 bits
#define SYMBOL_FG_SHIFTS			(21)
#define SYMBOL_FG_MASK				(0x7f<<SYMBOL_FG_SHIFTS)		// 7 bits
#define SYMBOL_BG_SHIFTS			(14)
#define SYMBOL_BG_MASK				(0x7f<<SYMBOL_BG_SHIFTS)		// 7 bits
#define SYMBOL_CP_SHIFTS			(0)
#define SYMBOL_CP_MASK				(0x3fff<<SYMBOL_CP_SHIFTS)		// 14 bits
#define SYMBOL_COLOR_BITS			7


#endif

#else

typedef uint64_t symbol_t;

// 64-bit layout
// ---- ---- --aa aaff ffff fffb bbbb bbbb ---- ---- ---- cccc cccc cccc cccc cccc
//  a = attributes
//  f = foreground
//  b = background
//  c = codepoint
#define SYMBOL_ATTRIBUTES_SHIFTS	(50)
#define SYMBOL_ATTRIBUTES_MASK		(0xfL<<SYMBOL_ATTRIBUTES_SHIFTS)	// 4 bits
#define SYMBOL_FG_SHIFTS			(32)
#define SYMBOL_FG_MASK				(0x1ffL<<SYMBOL_FG_SHIFTS)			// 9 bits
#define SYMBOL_BG_SHIFTS			(41)
#define SYMBOL_BG_MASK				(0x1ffL<<SYMBOL_BG_SHIFTS)			// 9 bits
#define SYMBOL_CP_SHIFTS			(0)
#define SYMBOL_CP_MASK				(0xfffff<<SYMBOL_CP_SHIFTS)			// 20 bits
#define SYMBOL_COLOR_BITS			9

#endif


// additional data layout:
// bits     987654321
// fg-color       xxx
// bg-color    xxx
typedef uint_fast8_t symbol_color_t;


#define TE_COLOR_TEXT_FG		0	///< default text foreground color
#define TE_COLOR_TEXT_BG		1	///< default text background color
#define TE_COLOR_MOUSE_FG		2	///< mouse cursor foreground color
#define TE_COLOR_MOUSE_BG		3	///< mouse cursor background color
#define TE_COLOR_SELECTION_FG	4	///< selection/highlight foreground color
#define TE_COLOR_SELECTION_BG	5	///< selection/highlight background color

#define TE_COLOR_ANSI			6	///< beginning of ANSI colors, corresponds to 0(BLACK)
#define TE_COLOR_BLACK			6	///< ANSI color 0
#define TE_COLOR_RED			7	///< ANSI color 1
#define TE_COLOR_GREEN			8	///< ANSI color 2
#define TE_COLOR_YELLOW			9	///< ANSI color 3
#define TE_COLOR_BLUE			10	///< ANSI color 4
#define TE_COLOR_MAGENTA		11	///< ANSI color 5
#define TE_COLOR_CYAN			12	///< ANSI color 6
#define TE_COLOR_GRAY			13	///< ANSI color 7

#if ((1<<SYMBOL_COLOR_BITS) - TE_COLOR_ANSI > 256)
#	define TE_COLOR_MODE	256
#elif ((1<<SYMBOL_COLOR_BITS) - TE_COLOR_ANSI > 88)
#	define TE_COLOR_MODE	88
#elif ((1<<SYMBOL_COLOR_BITS) - TE_COLOR_ANSI > 16)
#	define TE_COLOR_MODE	16
#elif ((1<<SYMBOL_COLOR_BITS) - TE_COLOR_ANSI > 8)
#	define TE_COLOR_MODE	8
#else
#	error "unsupported configuration"
#endif

#define TE_COLOR_COUNT	(TE_COLOR_ANSI+TE_COLOR_MODE)

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

/**
 * Swaps FG<->BG color of symbol
 */
static inline symbol_t symbol_swap_colors(symbol_t sym) {
	const symbol_color_t bg = (sym & SYMBOL_BG_MASK) >> SYMBOL_BG_SHIFTS;
	const symbol_color_t fg = (sym & SYMBOL_FG_MASK) >> SYMBOL_BG_SHIFTS;
	return	(sym & ~(SYMBOL_BG_MASK|SYMBOL_FG_MASK) )
	| ((symbol_t)bg << SYMBOL_FG_SHIFTS)
	| ((symbol_t)fg << SYMBOL_BG_SHIFTS);
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
