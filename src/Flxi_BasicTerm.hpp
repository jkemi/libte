#ifndef Flx_Term_H
#define Flx_Term_H

#include <iconv.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "LibTE.hpp"

#include "Flx_IResizableParent.hpp"
#include "Flx_IChildHandler.hpp"

namespace Flx {
namespace VT {
namespace impl {

/**
 * A FLTK widget that handles lower-level terminal tasks:
 *  - Rendering of terminal output
 *  - User input to terminal (keyboard, mouse)
 *  - Fonts
 *  - Resizing
 */
class BasicTerm : public Fl_Box, public LibTE
{
public:

	//
	// Public API
	//

	// Interface defining event handling for higher GUI functions.
	class IEventHandler : public IResizableParent {
	public:
		virtual void event_scrollposition(int offset, int size) = 0;
	};



public:
	// constructor
	// childh, eventh must be valid for the entire lifetime of Fl_Term
	BasicTerm(int fontsize, IChildHandler* childh, IEventHandler* eventh, int X, int Y, int W, int H);
	virtual ~BasicTerm();

	virtual void init();

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
	void printTerminalDebug(FILE* where);
	#endif


//
// Private member fields
//
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

	IChildHandler*	_child_handler;
	IEventHandler*	_event_handler;


//
// Private methods
//
private:
	int32_t			_fltkToCP(const char* text, size_t len);
	char			_cpToFltk(int32_t cp);
	bool		 	_handle_keyevent(void);
	static void		_s_deferred_update_cb(void* data) { ((BasicTerm*)data)->_deferred_update_cb();}
	void			_deferred_update_cb();

//
// These are TE_Frontend callback handlers
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
	void fe_set_clipboard (te_clipbuf_t clipbuf, const int32_t* text, int len);
	int fe_request_clipboard (te_clipbuf_t clipbuf, int32_t* text, int size);

};

} // namespace impl
} // namespace VT
} // namespace Flx

#endif // Flx_Term_H
