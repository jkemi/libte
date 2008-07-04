#ifndef BUFFERSYMBOL_H_
#define BUFFERSYMBOL_H_

#include <stdint.h>

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

// TODO: needed attributes
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

// TODO: don't ignore flags...
static inline symbol_t symbol_make_style(symbol_color_t fg, symbol_color_t bg, int flags) {
	return ((symbol_t)bg)<<25 | ((symbol_t)fg)<<22;
}

#endif