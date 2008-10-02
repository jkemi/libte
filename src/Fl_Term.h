#ifndef Flx_Term_H
#define Flx_Term_H

#include <iconv.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "LibTE.hpp"


/**
 * A FLTK widget that handles lower-level terminal tasks:
 *  - Rendering of terminal output
 *  - User input to terminal (keyboard, mouse)
 *  - Fonts
 *  - Resizing
 */
class Fl_Term : public Fl_Box, public TE
{
private:
	uint64_t	last_draw;

	// font stuff
	struct {
		int pixw;		// width of each font cell in pixels
		int pixh;		// height of each font cell in pixels
	} font;

	// draw stuff
	struct {
		int xoff;		// xoffset in pixels
		int yoff;		// yoffset in pixels
		int pixw;		// width in pixels
		int pixh;		// height in pixels
		int ncols;		// terminal width in chars
		int nrows;		// terminal height in chars
	} gfx;

	iconv_t		_fltk_to_cp;
	iconv_t		_cp_to_fltk;


//
// Public API
//

public:
	// constructor
	Fl_Term(int fs, int X, int Y, int W, int H, const char *L=0);
	virtual ~Fl_Term();


protected:
	// Overridden Fl_Widget
	void draw(void);

public:
	// Overridden Fl_Widget
	// handle keyboard etc.
	int handle(int);

	// Overridden Fl_Widget
	// handle window resizing
	void resize(int x, int y, int w, int h);


	#ifndef NDEBUG
	void debug(FILE* where);
	#endif

	void set_scroll_func(void (*funcptr)(void* priv, int offset, int size), void* priv) {_scroll_func = funcptr; _scroll_priv = priv;}
	void set_send_back_func(void (*funcptr)(void* priv, const int32_t* data, int len), void* priv) {_send_back_func = funcptr; _send_back_priv = priv;}
	void set_size_func(void (*funcptr)(void* priv, int width, int height), void* priv) {_size_func = funcptr; _size_priv = priv;}


private:
	int32_t			_fltkToCP(const char* text, size_t len);
	char			_cpToFltk(int32_t cp);
	bool		 	_handle_keyevent(void);

	void			(*_send_back_func)(void* priv, const int32_t* data, int len);
	void*			_send_back_priv;

	void			(*_scroll_func)(void* priv, int offset, int size);
	void*			_scroll_priv;

	void			(*_size_func)(void* priv, int width, int height);
	void*			_size_priv;

	static void		_s_deferred_update_cb(void* data) { ((Fl_Term*)data)->_deferred_update_cb();}

	void			_deferred_update_cb();

	void _sendBack(const int32_t* data, int len);
	void _scrollPosition(int offset, int size);
	void _termSize(int width, int height);


private:
	//
	// These are TE_Frontend callbacks
	//
	void fe_draw_text(int x, int y, const symbol_t* data, int len);
	void fe_draw_clear(int x, int y, const symbol_color_t bg_color, int len);
	void fe_draw_cursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp);
	void fe_draw_move(int y, int height, int byoffset);

	void fe_updated();
	void fe_reset();
	void fe_bell();
	void fe_title(const int32_t* text, int len);
	void fe_request_resize(int width, int height);
	void fe_position(int offset, int size);

	void fe_send_back(const int32_t* data, int len);
};



#endif // Flx_Term_H
