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
	int tw, th, fh, fw, fnt_desc;
	int crs_x, crs_y, crs_fg, crs_bg, crs_flags;
	float cw;
	int def_fnt_size;

	iconv_t		_fltk_to_cp;

private:
	int32_t			_fltkToCP(const char* text, size_t len);
	bool		 	_handle_keyevent(void);

	void			(*_send_back_func)(void* priv, const int32_t* data);
	void*			_send_back_priv;

	void			(*_scroll_func)(void* priv, int offset, int size);
	void*			_scroll_priv;

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

	// text box view width/height
	int get_w(void) {return tw;}
	int get_h(void) {return th;}

	// font handling
	void font_size(int sz) {def_fnt_size = sz;}

	void set_scroll_func(void (*funcptr)(void* priv, int offset, int size), void* priv) {_scroll_func = funcptr; _scroll_priv = priv;}
	void set_send_back_func(void (*funcptr)(void* priv, const int32_t* data), void* priv) {_send_back_func = funcptr; _send_back_priv = priv;}

	void			_sendBack(const int32_t* data);

	//TODO: remove me
	void 			_sendBackMBS(const char* data);

	void			_scrollPosition(int offset, int size);
};
/************************************************************************/


#endif // HAVE_FL_TERM_HDR

/* End of File */
