#ifndef HAVE_FL_TERM_HDR
#define HAVE_FL_TERM_HDR

#include <iconv.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "gterm/libte.h"


/************************************************************************/
// This class is what the world should use...
class Fl_Term : public Fl_Box
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

public:
	//
	// These are TE_Frontend methods and should be considered private
	//
	void _fe_Updated(void);
	void _fe_DrawStyledText(int x, int y, const symbol_t* symbols, int len);
	void _fe_ClearChars(symbol_color_t bg_color, int x, int y, int len);
	void _fe_Move(int y, int height, int byoffset);
	void _fe_DrawCursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp);


public:
	TE_Backend*	_te;

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

	void set_send_back_func(void (*funcptr)(void* priv, const int32_t* data), void* priv) {_send_back_func = funcptr; _send_back_priv = priv;}

	void			_sendBack(const int32_t* data);

	//TODO: remove me
	void 			_sendBackMBS(const char* data);

};
/************************************************************************/


#endif // HAVE_FL_TERM_HDR

/* End of File */
