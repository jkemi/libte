#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <alloca.h>
#include <sys/time.h>

#include "strutil.h"

#include "Flxi_BasicTerm.hpp"

#ifdef FLTE_ENABLE_FT
#	include "fontrender.h"
#endif

/*
 #define _DEFER_DRAWING_US	20000		// if drawing occured within this time from last draw, reset draw timer to _DEFERRED_DRAWING_DELAY
 #define _DEFERRED_DRAWING_DELAY	0.02	// delay (in seconds) until we perform actual drawing (if _DEFER_DRAWING_US matched)
 */

// TODO: this basically disables first param
#define _DEFER_DRAWING_US	2000000		// if drawing occured within this time from last draw, reset draw timer to _DEFERRED_DRAWING_DELAY
#define _DEFERRED_DRAWING_DELAY	(1.0/100)	// delay (in seconds) until we perform actual drawing (if _DEFER_DRAWING_US matched)


#ifndef FLTE_ENABLE_FT
#	define FONT_SIZE 14
#endif



namespace Flx {
namespace VT {
namespace impl {

// VT100 color table - map Colors to FL-colors:
static const Fl_Color col_table[] = {
	// NORMAL
	FL_BLACK,			// 0 BLACK
	FL_DARK_RED,		// 1 RED
	FL_DARK_GREEN,		// 2 GREEN
	FL_DARK_YELLOW,		// 3 YELLOW
	FL_DARK_BLUE,		// 4 BLUE
	FL_DARK_MAGENTA,	// 5 MAGENTA
	FL_DARK_CYAN,		// 6 CYAN
	FL_WHITE,			// 7 WHITE

	// "Bright"
	FL_GRAY0,
	FL_RED,
	FL_GREEN,
	FL_YELLOW,
	FL_BLUE,
	FL_MAGENTA,
	FL_CYAN,
	FL_WHITE,

	/*
	FL_BLACK,
	fl_rgb_color(139, 0, 0),		// RED
	fl_rgb_color(0, 139, 0),		// GREEN
	fl_rgb_color(139, 139, 0),		// YELLOW
	fl_rgb_color(0, 0, 139),		// BLUE
	fl_rgb_color(139, 0, 139),		// MAGENTA
	fl_rgb_color(0, 139, 139),		// CYAN
	fl_rgb_color(167, 167, 167),	// WHITE
	FL_DARK_BLUE,
	FL_DARK_CYAN,
	FL_DARK_RED,
	FL_DARK_YELLOW,
	FL_DARK_GREEN,
	FL_DARK_MAGENTA,*/
/*
	fl_rgb_color(0,0,0),		// DARK BLACK
	fl_rgb_color(133,0,11),		// DARK RED
	fl_rgb_color(39,152,5),		// DARK GREEN
	fl_rgb_color(136,136,4),	// DARK YELLOW
	fl_rgb_color(0,14,170),		// DARK BLUE
	fl_rgb_color(155,0,169),	// DARK MAGENTA
	fl_rgb_color(32,149,165),	// DARK CYAN
	fl_rgb_color(178,178,178),	// DARK WHITE

	fl_rgb_color(102,102,102),	// BLACK
	fl_rgb_color(216,0,6),		// RED
	fl_rgb_color(55,213,6),		// GREEN
	fl_rgb_color(225,226,9),	// YELLOW
	fl_rgb_color(0,20,255),		// BLUE
	fl_rgb_color(211,0,234),	// MAGENTA
	fl_rgb_color(0,224,255),	// CYAN
	fl_rgb_color(223,233,255),	// WHITE
*/
};

static const uint8_t col_palette[] = {
	// Normal
	0,0,0,			// BLACK
	128,0,0,		// RED
	0,128,0,		// GREEN
	128,128,0,		// YELLOW
	0,0,128,		// BLUE
	128,0,128,		// MAGENTA
	0,128,128,		// CYAN
	128,128,128,	// WHITE

	// Bright
	0,0,0,			// BLACK
	255,0,0,		// RED
	0,255,0,		// GREEN
	255,255,0,		// YELLOW
	0,0,255,		// BLUE
	255,0,255,		// MAGENTA
	0,255,255,		// CYAN
	255,255,255,	// WHITE

/*
	0,0,0,			// BLACK
	139,0,0,		// RED
	0,139,0,		// GREEN
	139,139,0,		// YELLOW
	0,0,139,		// BLUE
	139,0,139,		// MAGENTA
	0,139,139,		// CYAN
	167,167,167,	// WHITE
	0,0,128,		// DARK BLUE
	128,128,128,	// DARK CYAN
	128,0,0,		// DARK RED
	128,128,0,		// DARK YELLOW
	0,128,0,		// DARK GREEN
	128,0,128,		// DARK MAGENTA
*/

/*
	0,0,0,			// DARK BLACK
	133,0,11,		// DARK RED
	39,152,5,		// DARK GREEN
	136,136,4,		// DARK YELLOW
	0,14,140,		// DARK BLUE
	155,0,169,		// DARK MAGENTA
	32,149,165,		// DARK CYAN
	178,178,178,	// DARK WHITE
	102,102,102,	// BLACK
	216,0,6,		// RED
	55,213,6,		// GREEN
	225,226,9,		// YELLOW
	0,20,255,		// BLUE
	211,0,234,		// MAGENTA
	225,224,255,	// CYAN
	223,233,255,	// WHITE
*/
};


static uint64_t getCurrentTime_us(void) {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1) {
		// TODO: handle
	}

	return (uint64_t)tv.tv_sec * 1000000L + tv.tv_usec;
}

// Must only be called inside handle()
static te_modifier_t getTeKeyModifiers() {
	return (te_modifier_t)(
		(Fl::event_ctrl()	? TE_MOD_CTRL  : TE_MOD_NONE) |
// TODO: we need to detect altgr key from fltk somehow, since composed characters
// shouldn't set MOD_META, disable for now.
#ifndef __APPLE__
		(Fl::event_alt()	? TE_MOD_META  : TE_MOD_NONE) |
#endif
		(Fl::event_shift()	? TE_MOD_SHIFT : TE_MOD_NONE)
	);
}

/************************************************************************/
// This is the implementation of the user-facing parts of the widget...
/************************************************************************/
BasicTerm::BasicTerm (	int fontsize,
						SlaveIO* childh, IEventHandler* eventh,
						int X, int Y, int W, int H)
// Parent constructors
	: Fl_Box(X,Y,W,H,0)
	, LibTE()

{
	box(FL_THIN_DOWN_FRAME);


	// Size of cell in pixels
#ifdef FLTE_ENABLE_FT
	tr_init(col_palette);
	font.pixw = tr_width();
	font.pixh = tr_height();
#else
	fl_font(FL_COURIER, FONT_SIZE);
	font.pixw = (fl_width("MHW#i1l")/7.0f + 0.5f);
	font.pixh = fl_height();
	font.xoff = 0;
	font.yoff = fl_height()-fl_descent();
#endif


	// Size of graphics area in cells
	gfx.ncols = (w()-Fl::box_dw(box())) / font.pixw;
	gfx.nrows = (h()-Fl::box_dh(box())) / font.pixh;

	// Size of graphics area in pixels
	gfx.pixw = gfx.ncols*font.pixw;
	gfx.pixh = gfx.nrows*font.pixh;

	// Start of graphics area, relative widget
	gfx.xoff = Fl::box_dx(box()); // for center: ((w()-Fl::box_dw(box())) - gfx.pixw) / 2 + Fl::box_dx(box());
	gfx.yoff = Fl::box_dy(box()); // for center: ((h()-Fl::box_dh(box())) - gfx.pixh) / 2 + Fl::box_dy(box());

	_child_handler = childh;
	_event_handler = eventh;

	_child_handler->setHandler(&_s_handle_from_slave, this);
}

BasicTerm::~BasicTerm() {
#ifdef FLTE_ENABLE_FT
	tr_term();
#endif

//	delete _child_handler;
//	delete _event_handler;
}

void BasicTerm::init() {
	teInit(gfx.ncols, gfx.nrows, NULL, 0);
	_child_handler->resizeSlave(gfx.ncols, gfx.nrows);

	const int pixwidth = (gfx.ncols*font.pixw)+Fl::box_dw(box());
	const int pixheight = (gfx.nrows*font.pixh)+Fl::box_dh(box());

	printf("requested size %d %d\n", gfx.ncols, gfx.nrows);
	_event_handler->event_want_size(pixwidth, pixheight);

	// Size limits are determined by:
	//  - Height at least 2 lines (for scroll margins)
	//  - Width at least 4 (for wrapping, bs etc..)
	//  - Width and height must be max 256-32-1=233 for mouse report to work correctly
	const int minw = (4)*font.pixw + Fl::box_dw(box());
	const int maxw = (256-32+1)*font.pixw+Fl::box_dw(box());

	const int minh = (2)*font.pixh + Fl::box_dh(box());
	const int maxh = (256-32+1)*font.pixh+Fl::box_dh(box());

	_event_handler->event_size_range(minw, minh, maxw, maxh, font.pixw, font.pixh);
}

int32_t BasicTerm::_s_fltkToCP(const char* text, size_t len) {
	int32_t ret;
	if (len == 0) {
		ret = 0xFFFD;				// unicode REPLACEMENT CHARACTER
	} else if (*text & 0x80) {              // what should be a multibyte encoding
		int l;
		ret = fl_utf8decode(text,text+len,&l);
		if (l<2) {
			ret = 0xFFFD;   // Turn errors into REPLACEMENT CHARACTER
		}
    } else {                      // handle the 1-byte utf8 encoding:
		ret = *text;
    }
	return ret;
}
	
int32_t* BasicTerm::_s_fltkToCPs(const char* text, size_t bytes, size_t* cplen) {
	// TODO: size
	int32_t* cps = (int32_t*)malloc(sizeof(int32_t)*Fl::event_length());
	if (cps == NULL) {
		*cplen = 0;
		return NULL;
	}
	
	const char* src = text;
	int32_t* dest = cps;
	size_t len = 0;
	
	while (bytes > 0) {
		if (*src & 0x80) {              // what should be a multibyte encoding
			int l;
			*dest = fl_utf8decode(src,src+bytes,&l);
			if (l<2) {
				*dest = 0xFFFD;   // Turn errors into REPLACEMENT CHARACTER
			}
			bytes-=l;
			src+=l;
		} else {                      // handle the 1-byte utf8 encoding:
			*dest = *src;
			bytes--;
			src++;
		}
		dest++;
		len++;
	}

	*cplen = len;
	return cps;
}

bool BasicTerm::_handle_keyevent(void) {
	const int keysym = Fl::event_key();

	te_key_t tekey = TE_KEY_UNDEFINED;

	switch (keysym) {
	case FL_Enter:		tekey = TE_KEY_ENTER;		break;
	case FL_Tab:		tekey = TE_KEY_TAB;			break;
	case FL_Escape:		tekey = TE_KEY_ESCAPE;		break;
	case FL_BackSpace:	tekey = TE_KEY_BACKSPACE;	break;

	case FL_Home:		tekey = TE_KEY_HOME;	break;
	case FL_Insert:		tekey = TE_KEY_INSERT;	break;
	case FL_Delete:		tekey = TE_KEY_DELETE;	break;
	case FL_End:		tekey = TE_KEY_END;		break;
	case FL_Page_Up:	tekey = TE_KEY_PGUP;	break;
	case FL_Page_Down:	tekey = TE_KEY_PGDN;	break;

	case FL_Left:		tekey = TE_KEY_LEFT;	break;
	case FL_Right:		tekey = TE_KEY_RIGHT;	break;
	case FL_Up:			tekey = TE_KEY_UP;		break;
	case FL_Down:		tekey = TE_KEY_DOWN;	break;

	case FL_KP+'=':		tekey = TE_KP_EQUAL;		break;
	case FL_KP+'/':		tekey = TE_KP_DIVIDE;		break;
	case FL_KP+'*':		tekey = TE_KP_MULTIPLY;		break;
	case FL_KP+'-':		tekey = TE_KP_SUBSTRACT;	break;
	case FL_KP+'+':		tekey = TE_KP_ADD;			break;
	case FL_KP+'.':		tekey = TE_KP_PERIOD;		break;
	case FL_KP+',':		tekey = TE_KP_COMMA;		break;
	case FL_KP+'\r':	tekey = TE_KP_ENTER;		break;

	case FL_KP+'0':	tekey = TE_KP_0;	break;
	case FL_KP+'1':	tekey = TE_KP_1;	break;
	case FL_KP+'2':	tekey = TE_KP_2;	break;
	case FL_KP+'3':	tekey = TE_KP_3;	break;
	case FL_KP+'4':	tekey = TE_KP_4;	break;
	case FL_KP+'5':	tekey = TE_KP_5;	break;
	case FL_KP+'6':	tekey = TE_KP_6;	break;
	case FL_KP+'7':	tekey = TE_KP_7;	break;
	case FL_KP+'8':	tekey = TE_KP_8;	break;
	case FL_KP+'9':	tekey = TE_KP_9;	break;
	}

	if (keysym >= FL_F+1 && keysym <= FL_F_Last) {
		tekey = (te_key_t)(TE_KEY_F + keysym-FL_F);
	}

	if (tekey != TE_KEY_UNDEFINED) {
		te_modifier_t mod = getTeKeyModifiers();
		return teHandleButton(tekey, mod) != 0;
	}

	if (Fl::event_length() > 0) {

		/*
		int ndeleted;
		if (Fl::compose(ndeleted) == false) {
			printf("compose() == false, %d\n", ndeleted);
			return NULL;
		}
		*/

		const int32_t cp = _s_fltkToCP(Fl::event_text(), Fl::event_length());

		te_modifier_t mod = getTeKeyModifiers();

#ifndef NDEBUG
		printf("cp was: %d mod was %d\n", cp, mod);
#endif

		if (cp >= 0) {
			teHandleKeypress(cp, mod);
			return true;
		}
	}

	return false;
}
	
bool BasicTerm::_handle_pasteevent(void) {
	fprintf(stderr, "got paste");

	size_t len;
	int32_t* cps = _s_fltkToCPs(Fl::event_text(), Fl::event_length(), &len);
	if (cps != NULL) {
		tePasteText(cps, len);
		free(cps);
		return true;
	}
	return false;
}

void BasicTerm::_deferred_update_cb() {
	damage(FL_DAMAGE_USER1);
}

void BasicTerm::_handle_from_slave(const int32_t *data, size_t len) {
	// Process exited!
	if (len == 0) {
		int exit_status = *data;
		_event_handler->event_childexit(exit_status);
		return;
	}
	teProcessInput(data, len);
}

/************************************************************************/
// handle keyboard focus etc
int BasicTerm::handle(int event)
{
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	case FL_KEYDOWN:
		if (_handle_keyevent()) {
			return 1;
		}
		break;
	
	case FL_PASTE:
		if (_handle_pasteevent()) {
			return 1;
		}
		break;

	//
	// Mouse handling
	//
	case FL_ENTER:	// Mouse enters
	case FL_LEAVE:	// Mouse leaves
		return 1;
	case FL_PUSH:
	case FL_DRAG:
	case FL_RELEASE:
	case FL_MOUSEWHEEL:
	case FL_MOVE: {
		// Make sure that we're inside terminal area
		if ( Fl::event_inside(x()+gfx.xoff, y()+gfx.yoff, gfx.pixw, gfx.pixh) ) {
			int cellx = (Fl::event_x() - x()-gfx.xoff) / font.pixw;
			int celly = (Fl::event_y() - y()-gfx.yoff) / font.pixh;

			te_modifier_t mod =	getTeKeyModifiers();

			te_mouse_button_t btn = (te_mouse_button_t)(
				(Fl::event_button1() ? TE_MOUSE_LEFT   : TE_MOUSE_NONE) |
				(Fl::event_button2() ? TE_MOUSE_MIDDLE : TE_MOUSE_NONE) |
				(Fl::event_button3() ? TE_MOUSE_RIGHT  : TE_MOUSE_NONE) |
				((event==FL_MOUSEWHEEL && Fl::event_dy() <0) ? TE_MOUSE_WHEEL_UP : TE_MOUSE_NONE) |
				((event==FL_MOUSEWHEEL && Fl::event_dy() >0) ? TE_MOUSE_WHEEL_DOWN : TE_MOUSE_NONE) |
				((event==FL_PUSH && Fl::event_clicks() > 0) ? TE_MOUSE_DOUBLE : TE_MOUSE_NONE) |
				((event==FL_PUSH && Fl::event_clicks() > 1) ? TE_MOUSE_TRIPLE : TE_MOUSE_NONE)
			);

			if (btn & TE_MOUSE_TRIPLE) {
				Fl::event_clicks(0);
			}


//			printf("clicks: %d\n", Fl::event_clicks());

			//printf("%d,%d %x %x\n", cellx, celly, btn, mod);
			teHandleMouse(cellx, celly, btn, mod);
			return 1;
		}
		break;
	}
	default:
		break;
	}
	
	return Fl_Box::handle(event);
}

/************************************************************************/
void BasicTerm::resize(int x, int y, int W, int H)
{
	Fl_Box::resize(x, y, W, H);

	gfx.ncols = (w()-Fl::box_dw(box())) / font.pixw;
	gfx.nrows = (h()-Fl::box_dh(box())) / font.pixh;

	if (gfx.ncols != teGetWidth() || gfx.nrows != teGetHeight()) {
		if (!_child_handler->resizeSlave(gfx.ncols, gfx.nrows)) {
			printf("not able to resize child\n");
			return;
		}

		teResize(gfx.ncols, gfx.nrows);

		gfx.ncols = teGetWidth();
		gfx.nrows = teGetHeight();
		gfx.pixw = gfx.ncols*font.pixw;
		gfx.pixh = gfx.nrows*font.pixh;
		gfx.xoff = Fl::box_dx(box()); // ((w()-Fl::box_dw(box())) - gfx.pixw) / 2 + Fl::box_dx(box());
		gfx.yoff = Fl::box_dy(box()); // ((h()-Fl::box_dh(box())) - gfx.pixh) / 2 + Fl::box_dy(box());


		printf("terminal resized to: %d, %d\n", gfx.ncols, gfx.nrows);
	}
}

/************************************************************************/
#ifndef NDEBUG
void BasicTerm::printTerminalDebug(FILE* where) {
	teDebug(where);
}
#endif

/************************************************************************/
void BasicTerm::draw(void)
{
	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());
	const int wd = w() - Fl::box_dw(this->box());
	const int ht = h() - Fl::box_dh(this->box());
	
	Fl_Box::draw();
	fl_push_clip(xo, yo, wd, ht);

	const bool forced = ((damage() & FL_DAMAGE_ALL) != 0);
	if (forced) {
		printf("forced redraw: 0x%02x.\n", damage());
		fl_rectf(xo, yo, wd, ht, 0, 0, 0);
	}

	teRequestRedraw(0, 0, gfx.ncols, gfx.nrows, forced);

	// restore the clipping rectangle...
	fl_pop_clip();

	last_draw = getCurrentTime_us();
}


//////////////////////////////////////////////////////////////////////
// TE_Frontend methods
//////////////////////////////////////////////////////////////////////

void BasicTerm::fe_draw_text(int xpos, int ypos, const symbol_t* symbols, int len) {
	const int xo = x() + gfx.xoff;
	const int yo = y() + gfx.yoff;

#ifndef NDEBUG
	if (len > 4) {
		//printf("DrawText(): %d, %d (%d))\n", xpos, ypos, len);
	}
#endif

	int xp = xo + xpos*font.pixw;
	const int yp = yo + ypos*font.pixh;

	for (int i = 0; i < len; i++) {
		const symbol_t sym = symbols[i];
		const int cp = symbol_get_codepoint(sym);

#ifdef FLTE_ENABLE_FT

		if (cp == ' ') {
			const symbol_attributes_t attrs = symbol_get_attributes(sym);
			symbol_color_t bg_color = symbol_get_bg(sym);
			if (attrs & SYMBOL_INVERSE) {
				bg_color = symbol_get_fg(sym);
			}

			Fl_Color bg = col_table[bg_color];
			fl_color(bg);
			fl_rectf(xp, yp, font.pixw, font.pixh);
		} else {
			const symbol_attributes_t attrs = symbol_get_attributes(sym);
			symbol_color_t fg_color = symbol_get_fg(sym);
			if (attrs & SYMBOL_INVERSE) {
				fg_color = symbol_get_bg(sym);
			}

			const uint8_t* fontdata = tr_get(sym);
			fl_draw_image(fontdata, xp, yp, font.pixw, font.pixh, 3);
		}
#else
		const symbol_attributes_t attrs = symbol_get_attributes(sym);
		symbol_color_t bg_color = symbol_get_bg(sym);
		symbol_color_t fg_color = symbol_get_fg(sym);
		if (attrs & SYMBOL_INVERSE) {
			symbol_color_t tmp = fg_color;
			fg_color = bg_color;
			bg_color = tmp;
		}

		Fl_Color bg = col_table[bg_color];
		fl_color(bg);
		fl_rectf(xp, yp, font.pixw, font.pixh);

		if (cp != ' ') {
			Fl_Color fg = col_table[fg_color];
			char buf[6];
			const int l = fl_utf8encode(cp, buf);
			buf[l] = '\0';
			fl_color(fg);

			// BOLD and BLINK
			if ((attrs & (SYMBOL_BOLD|SYMBOL_BLINK)) == (SYMBOL_BOLD|SYMBOL_BLINK)) {
				fl_font(FL_COURIER_BOLD_ITALIC, FONT_SIZE);
			} else if (attrs & SYMBOL_BOLD) {
				fl_font(FL_COURIER_BOLD, FONT_SIZE);
			} else if (attrs & SYMBOL_BLINK) {
				fl_font(FL_COURIER_ITALIC, FONT_SIZE);
			} else {
				fl_font(FL_COURIER, FONT_SIZE);
			}

			fl_draw(buf, l, xp+font.xoff, yp+font.yoff);
		}
#endif
		xp += font.pixw;
	}
}

void BasicTerm::fe_draw_clear(int xpos, int ypos, const symbol_color_t bg_color, int len) {
	//	printf("DrawClear: %d, %d (%d))\n", xpos, ypos, len);

	const int xo = x() + gfx.xoff;
	const int yo = y() + gfx.yoff;

	int xp = xo + xpos*font.pixw;
	const int yp = yo + ypos*font.pixh;

	assert (bg_color >= 0 && bg_color <= 7);
	Fl_Color bg = col_table[bg_color];

	fl_color(bg);
	fl_rectf(xp, yp, font.pixw*len, font.pixh);
}

void BasicTerm::fe_draw_cursor(int xpos, int ypos, symbol_t symbol) {
	fe_draw_text(xpos, ypos, &symbol, 1);
	return;
	
	/*
	const int xp = x() + gfx.xoff + font.pixw * xpos;
	const int yp = y() + gfx.yoff + font.pixh * ypos;

	assert (fg_color >= 0 && fg_color <= 7);

	if (attrs & SYMBOL_INVERSE) {
		symbol_color_t tmp = fg_color;
		fg_color = bg_color;
		bg_color = tmp;
	}
	
	const Fl_Color fg = col_table[fg_color];

	// now draw a simple box cursor
	fl_color(fg);
	fl_rectf(xp, yp, font.pixw, font.pixh);
	 */
}

void BasicTerm::fe_draw_move(int y, int height, int byoffset) {
	// TODO: implement
}

void BasicTerm::fe_updated() {
	const uint64_t now = getCurrentTime_us();
	if (now-last_draw < _DEFER_DRAWING_US) {
		Fl::remove_timeout(&_s_deferred_update_cb, this);
		Fl::add_timeout(_DEFERRED_DRAWING_DELAY, &_s_deferred_update_cb, this);
		return;
	}
	damage(FL_DAMAGE_USER1);
}

void BasicTerm::fe_reset() {
	// TODO: implement
}

void BasicTerm::fe_bell() {
	fl_beep();
}

void BasicTerm::fe_title(const int32_t* text, int len) {
	_event_handler->event_title(text, len);
}

void BasicTerm::fe_send_back(const int32_t* data, int len) {
	_child_handler->toSlave(data, len);
}

void BasicTerm::fe_request_resize(int width, int height) {
	int pixwidth = (width*font.pixw)+Fl::box_dw(box());
	int pixheight = (height*font.pixh)+Fl::box_dh(box());

	if (w() != pixwidth || h() != pixheight) {
		_event_handler->event_want_size(pixwidth, pixheight);
	}
}

void BasicTerm::fe_position(int offset, int size) {
	_event_handler->event_scrollposition(offset, size);
}

void BasicTerm::fe_set_clipboard (te_clipbuf_t clipbuf, const int32_t* text, int len) {
	int clipboard;
	switch(clipbuf) {
	case TE_CLIPBUF_PRIMARY:		clipboard = 0;	break;
	case TE_CLIPBUF_CLIPBOARD:		clipboard = 1;	break;
	default:
		return;
	}

	// TODO: redo!
	size_t nbytes = MB_CUR_MAX*(len+1);
	char tmp[nbytes];

	size_t nwritten;
	str_cps_to_mbs_n(tmp, text, nbytes, len, &nwritten, NULL);
	tmp[nwritten] = '\0';

	Fl::copy(tmp, clipboard);
}

void* BasicTerm::fe_request_clipboard (te_clipbuf_t clipbuf, int32_t* const* text, int* size) {
	// TODO: implement?
	return 0;
}

void BasicTerm::fe_request_clipboard_done (void* token) {
	// TODO: implement?
}



}	// namespace impl
}	// namespace VT
}	// namespace Flx
