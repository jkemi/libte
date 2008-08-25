#ifndef HAVE_FL_TERM_HDR
#define HAVE_FL_TERM_HDR

#include <iconv.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "LibTE.hpp"


/************************************************************************/
// This class is what the world should use...
class Fl_Term : public Fl_Box, public TE
{
protected:
	void draw(void);

	int def_fnt_size;

	// font stuff
	struct {
		int pixw;		// width of each font cell in pixels
		int pixh;		// height of each font cell in pixels
		int descent;	// TODO
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

private:
	int32_t			_fltkToCP(const char* text, size_t len);
	bool		 	_handle_keyevent(void);

	void			(*_send_back_func)(void* priv, const int32_t* data);
	void*			_send_back_priv;

	void			(*_scroll_func)(void* priv, int offset, int size);
	void*			_scroll_priv;

	void			(*_size_func)(void* priv, int width, int height);
	void*			_size_priv;

public:
	//
	// These are TE_Frontend methods and should be considered private
	//
	void fe_draw_text(int x, int y, const symbol_t* data, int len);
	void fe_draw_clear(int x, int y, const symbol_color_t bg_color, int len);
	void fe_draw_cursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp);
	void fe_draw_move(int y, int height, int byoffset);

	void fe_updated();
	void fe_reset();
	void fe_bell();
	void fe_mouse(int x, int y);
	void fe_title(const int32_t* text);
	void fe_send_back(const int32_t* data);
	void fe_request_resize(int width, int height);
	void fe_position(int offset, int size);


public:
	// constructor
	Fl_Term(int fs, int X, int Y, int W, int H, const char *L=0);
	virtual ~Fl_Term();

	// handle keyboard etc.
	int handle(int);

	// handle window resizing (NOT WORKING YET)
	void resize(int x, int y, int w, int h);

	// font handling
	void font_size(int sz) {def_fnt_size = sz;}

	void set_scroll_func(void (*funcptr)(void* priv, int offset, int size), void* priv) {_scroll_func = funcptr; _scroll_priv = priv;}
	void set_send_back_func(void (*funcptr)(void* priv, const int32_t* data), void* priv) {_send_back_func = funcptr; _send_back_priv = priv;}
	void set_size_func(void (*funcptr)(void* priv, int width, int height), void* priv) {_size_func = funcptr; _size_priv = priv;}

	void			_sendBack(const int32_t* data);
	void			_scrollPosition(int offset, int size);
	void			_termSize(int width, int height);
};
/************************************************************************/


#endif // HAVE_FL_TERM_HDR

/* End of File */
