#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "fontrender.h"

#define WIDTH	9
#define HEIGHT	15

typedef struct {
	symbol_t	symbol;
	uint8_t		data[];
} CacheElem;

static struct {
	int			size;
	int			capacity;
	CacheElem**	data;
} _cache;

static const uint8_t* _palette;

static void _cache_init(void) {
	_cache.size = 0;
	_cache.capacity = 64;
	_cache.data = (CacheElem**)malloc(sizeof(CacheElem*) * 64);
}

static void _cache_term(void) {
	for (int i = 0; i < _cache.size; i++) {
		free(_cache.data[i]);
	}
	free(_cache.data);
}

static CacheElem* _cache_lookup(symbol_t sym) {
	for (int i = 0; i < _cache.size; i++) {
		if (_cache.data[i]->symbol == sym) {
			return _cache.data[i];
		}
	}
	return NULL;
}

static void _cache_insert(CacheElem* elem) {
	if (_cache.size == _cache.capacity) {
		const int newcapacity = _cache.capacity*2;
		_cache.data = (CacheElem**)realloc(_cache.data, sizeof(CacheElem*) * newcapacity);
		_cache.capacity = newcapacity;
	}

	_cache.data[_cache.size++] = elem;
}

static FT_Library	_library;

static FT_Face _normal;
static FT_Face _bold;

static void _fatal(const char* format, ...) {
	// FIXME: remove all references to this method
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void _load_font(const char* path, FT_Face* face) {
	FT_Error error;

	error = FT_New_Face( _library, path, 0, face );
	if ( error) {
		_fatal("unable to load font\n");
	}

	// TODO: this was old block, maybe use FT_Set_Pixel_Size()?
/*	error = FT_Set_Char_Size(
			*face,		// handle to face object
			0,			// char_width in 1/64th of points
			WIDTH*64,	// char_height in 1/64th of points
			0,			// horizontal device resolution
			0 );
	if (error) {
		_fatal("unable to set char size\n");
	}*/

	if ( !FT_HAS_FIXED_SIZES((*face)) ) {
		_fatal("not a fixed-size font\n");
	}


	error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
	if (error) {
		_fatal("unable to select charmap\n");
	}
}

static void _render_glyph(CacheElem* dest, symbol_t sym) {
	FT_Error error;

	const int cp = symbol_get_codepoint(sym);
	const symbol_attributes_t attrs = symbol_get_attributes(sym);

	symbol_color_t fg_color = symbol_get_fg(sym);
	symbol_color_t bg_color = symbol_get_bg(sym);

	// Swap FG/BG colors if inverse
	if (attrs & SYMBOL_INVERSE) {
		symbol_color_t tmp = fg_color;
		fg_color = bg_color;
		bg_color = tmp;
	}

	FT_Face	face;
	bool	slanted = false;	// italic
	bool	underlined = false;

	if ((attrs & (SYMBOL_BOLD|SYMBOL_BLINK)) == (SYMBOL_BOLD|SYMBOL_BLINK)) {
		face = _bold;
		slanted = true;
		underlined = true;
	} else if (attrs & SYMBOL_BOLD) {
		face = _bold;
	} else if (attrs & SYMBOL_BLINK) {
		face = _normal;
		slanted = true;
		underlined = true;
	} else {
		face = _normal;
	}
	if (attrs & SYMBOL_UNDERLINE) {
		underlined = true;
	}

	error = FT_Load_Char(face, cp, FT_LOAD_RENDER);
	if (error) {
		_fatal("unable to render glyph\n");
	}

	const FT_Bitmap* monobmp = &face->glyph->bitmap;

	FT_Bitmap bmp;
	FT_Bitmap_New(&bmp);

	FT_Bitmap_Convert(_library, monobmp, &bmp, 1);

	const int8_t fg_r = _palette[fg_color*3+0];
	const int8_t fg_g = _palette[fg_color*3+1];
	const int8_t fg_b = _palette[fg_color*3+2];

	const int8_t bg_r = _palette[bg_color*3+0];
	const int8_t bg_g = _palette[bg_color*3+1];
	const int8_t bg_b = _palette[bg_color*3+2];

	for (int y = 0; y < bmp.rows; y++) {
		const uint8_t* srcrow = (const uint8_t*)bmp.buffer + y*bmp.pitch;
		uint8_t* destrow = dest->data + y*3*bmp.width;
		for (int x = 0; x < bmp.width; x++) {
			const uint16_t mixfactor = ((uint16_t)srcrow[x]*256) / (bmp.num_grays-1);

			destrow[x*3+0] = (fg_r*mixfactor + bg_r*(256-mixfactor)) >> 8;	// r
			destrow[x*3+1] = (fg_g*mixfactor + bg_g*(256-mixfactor)) >> 8;	// g
			destrow[x*3+2] = (fg_b*mixfactor + bg_b*(256-mixfactor)) >> 8;	// b
		}
	}
	if (underlined) {
		uint8_t* destrow = dest->data + (bmp.rows-1)*3*bmp.width;
		for (int x = 0; x < bmp.width; x++) {
			destrow[x*3+0] = fg_r;	// r
			destrow[x*3+1] = fg_g;	// g
			destrow[x*3+2] = fg_b;	// b
		}
	}


	FT_Bitmap_Done(_library, &bmp);
}

void tr_init(const uint8_t* palette) {
	_palette = palette;
	FT_Error error = FT_Init_FreeType(&_library);
	if (error) {
		_fatal("unable to initialize freetype\n");
	}

	// FIXME: setup via Fl_Preferemces ? or define perhaps?
#ifdef __APPLE__
	_load_font("test/9x15.pcf.gz", &_normal);
	_load_font("test/9x15B.pcf.gz", &_bold);
#else
	_load_font("/usr/share/fonts/X11/misc/9x15.pcf.gz", &_normal);
	_load_font("/usr/share/fonts/X11/misc/9x15B.pcf.gz", &_bold);
#endif

	_cache_init();
}

void tr_term(void) {
	_cache_term();

	FT_Done_Face(_bold);
	FT_Done_Face(_normal);
	FT_Done_FreeType(_library);
}

int tr_width(void) {
	return WIDTH;
}

int tr_height(void) {
	return HEIGHT;
}

const uint8_t* tr_get(symbol_t sym) {
	CacheElem* elem = _cache_lookup(sym);
	if (elem == NULL) {
		elem = (CacheElem*)malloc(sizeof(CacheElem)+WIDTH*HEIGHT*3);
		elem->symbol = sym;
		_render_glyph(elem, sym);
		_cache_insert(elem);
	}

	return elem->data;
}




