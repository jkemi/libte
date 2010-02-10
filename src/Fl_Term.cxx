#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <alloca.h>
#include <sys/time.h>

#include "strutil.h"

#include "Fl_Term.h"

// VT100 color table - map Colors to FL-colors:
static Fl_Color col_table[] = {
	FL_BLACK,
	FL_RED,
	FL_GREEN,
	FL_YELLOW,
	FL_BLUE,
	FL_MAGENTA,
	FL_CYAN,
	FL_WHITE,
	FL_DARK_BLUE,
	FL_DARK_CYAN,
	FL_DARK_RED,
	FL_DARK_YELLOW,
	FL_DARK_GREEN,
	FL_DARK_MAGENTA
};

#define _DEFER_DRAWING_US	20000
#define _DEFERRED_DRAWING_DELAY	0.02

static uint64_t getCurrentTime_us(void) {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1) {
		// TODO: handle
	}

	return (uint64_t)tv.tv_sec * 1000000L + tv.tv_usec;
}

/************************************************************************/
// This is the implementation of the user-facing parts of the widget...
/************************************************************************/
Fl_Term::Fl_Term(int sz, int X, int Y, int W, int H, const char *L) : Fl_Box(X,Y,W,H,L), TE()
{
	box(FL_THIN_DOWN_FRAME);

	def_fnt_size = sz;
	fl_font(FL_COURIER, def_fnt_size);

	const double cw = fl_width("MHW#i1l") / 7; // get an average char width, in case of Prop Fonts!
	font.pixh = fl_height();
	font.pixw = (int)(cw + 0.5);
	font.descent = fl_descent();

	gfx.ncols = (w()-Fl::box_dw(box())) / font.pixw;
	gfx.nrows = (h()-Fl::box_dh(box())) / font.pixh;
	gfx.pixw = gfx.ncols*font.pixw;
	gfx.pixh = gfx.nrows*font.pixh;
	gfx.xoff = Fl::box_dx(box()); // ((w()-Fl::box_dw(box())) - gfx.pixw) / 2 + Fl::box_dx(box());
	gfx.yoff = Fl::box_dy(box()); // ((h()-Fl::box_dh(box())) - gfx.pixh) / 2 + Fl::box_dy(box());

	_send_back_func = 0;
	_send_back_priv = 0;
	_scroll_func = 0;
	_scroll_priv = 0;

	teInit(gfx.ncols, gfx.nrows);

	// TODO: something weird here:
	// FLTK-1.1 _should_ really return iso8859-1 in event_text() but gives us UTF8 instead
	// also we don't want UCS-4LE on a big-endian machine...
#ifdef __APPLE__
	_fltk_to_cp = iconv_open("UCS-4LE", "MACROMAN");
	_cp_to_fltk = iconv_open("MACROMAN", "UCS-4LE");
#else
	_fltk_to_cp = iconv_open("UCS-4LE", "UTF8");
	_cp_to_fltk = iconv_open("UTF8", "UCS-4LE");
#endif
	if ((_fltk_to_cp == (iconv_t)-1) || _cp_to_fltk == (iconv_t)-1) {
		// TODO: handle somehow
		exit(EXIT_FAILURE);
	}
}

Fl_Term::~Fl_Term() {
	iconv_close(_fltk_to_cp);
	iconv_close(_cp_to_fltk);
}

int32_t Fl_Term::_fltkToCP(const char* text, size_t len) {
	str_mbs_hexdump("in: ", text, len);

/*	int deleted;
	if (Fl::compose(deleted) == false) {
		return -1;
	}

	printf ("deleted: %d\n", deleted);*/

	char* inbuf = (char*)text;
	size_t inleft = len;

	int32_t cp = -1;
	char* outbuf = (char*)&cp;
	size_t outleft = sizeof(int32_t);

	// reinitialize iconv state
	iconv(_fltk_to_cp, NULL, NULL, NULL, NULL);

	size_t result = iconv(_fltk_to_cp, &inbuf, &inleft, &outbuf, &outleft);

	if (result == (size_t)-1) {
		int err = errno;
		printf("not good: %d %s\n", err, strerror(err));
		return -1;
	}

	size_t written = (size_t)((uintptr_t)outbuf - (uintptr_t)&cp);

#ifndef NDEBUG
	printf ("result: %d %p, %p: %d\n", (int)result, &cp, outbuf, (int)written);
#endif

	if (written > 0) {
		return cp;
	} else {
		return -1;
	}
}

char Fl_Term::_cpToFltk(int32_t cp) {
	char* inbuf = (char*)&cp;
	size_t inleft = sizeof(int32_t);

	char c = 0;
	char* outbuf = &c;
	size_t outleft = 1;

	// reinitialize iconv state
	iconv(_cp_to_fltk, NULL, NULL, NULL, NULL);

	size_t result = iconv(_cp_to_fltk, &inbuf, &inleft, &outbuf, &outleft);

	if (result == (size_t)-1) {
		int err = errno;
		printf("not good: %d %s\n", err, strerror(err));
		return -1;
	}

	size_t written = (size_t)((uintptr_t)outbuf - (uintptr_t)&c);

#ifndef NDEBUG
	printf ("result: %d %p, %p: %d\n", (int)result, &c, outbuf, (int)written);
#endif

	if (written > 0) {
		return c;
	} else {
		return 0;
	}
}

bool Fl_Term::_handle_keyevent(void) {
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
		return teHandleButton(tekey) != 0;
	}

	if (Fl::event_length() > 0) {

		/*
		int ndeleted;
		if (Fl::compose(ndeleted) == false) {
			printf("compose() == false, %d\n", ndeleted);
			return NULL;
		}
		*/

		const int32_t cp = _fltkToCP(Fl::event_text(), Fl::event_length());

#ifndef NDEBUG
		printf("cp was: %d\n", cp);
#endif

		int mod = TE_MOD_NONE;
		if (Fl::event_alt()) {
			mod |= TE_MOD_META;
		}
		if (Fl::event_ctrl()) {
			mod |= TE_MOD_CTRL;
		}
		if (Fl::event_shift()) {
			mod |= TE_MOD_SHIFT;
		}

		if (cp >= 0) {
			teHandleKeypress(cp, (te_modifier_t)mod);
			return true;
		}
	}

	return false;
/*
	const int keylen = Fl::event_length();

	if (el) {
		key_c = Fl::event_text()[0]; // probably a dodgy assumption...
	}
	if((key >= ' ') && (key <= '~')) { // simple ASCII
		if ((key >= 'a') && (key <= 'z')) {
			if (Fl::event_ctrl()) { // ctrl-key
				key_c = key - 0x60;
			}
		// Also need to do something about ALT and META at this point...
		}
		txt[0] = key_c & 0x7F;
		te->SendBack(txt);
		return 1;
	}
	return 1; // return 1 for FL_KEYBOARD even if we don't know what they key is for!
	*/

}

void Fl_Term::_sendBack(const int32_t* data) {
	if (_send_back_func != 0) {
		_send_back_func(_send_back_priv, data);
	}
}

void Fl_Term::_scrollPosition(int offset, int size) {
	if (_scroll_func != 0) {
		_scroll_func(_scroll_priv, offset, size);
	}
}

void Fl_Term::_termSize(int width, int height) {
	if (_size_func != 0) {
		_size_func(_size_priv, width, height);
	}
}

void Fl_Term::_deferred_update_cb() {
	damage(FL_DAMAGE_USER1);
}

/************************************************************************/
// handle keyboard focus etc
int Fl_Term::handle(int event)
{
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	case FL_KEYBOARD: {
		if (_handle_keyevent()) {
			return 1;
		}
		break;
	}
	default:
		break;
	}

	return 0;
}

/************************************************************************/
// handle window resizing - THIS DOES NOT WORK RIGHT!
void Fl_Term::resize(int x, int y, int W, int H)
{
	Fl_Box::resize(x, y, W, H);

	gfx.ncols = (w()-Fl::box_dw(box())) / font.pixw;
	gfx.nrows = (h()-Fl::box_dh(box())) / font.pixh;

	if (gfx.ncols != teGetWidth() || gfx.nrows != teGetHeight()) {
		// Then tell the GTerm the new character sizes sizes...
		teResize(gfx.ncols, gfx.nrows);

		gfx.ncols = teGetWidth();
		gfx.nrows = teGetHeight();
		gfx.pixw = gfx.ncols*font.pixw;
		gfx.pixh = gfx.nrows*font.pixh;
		gfx.xoff = Fl::box_dx(box()); // ((w()-Fl::box_dw(box())) - gfx.pixw) / 2 + Fl::box_dx(box());
		gfx.yoff = Fl::box_dy(box()); // ((h()-Fl::box_dh(box())) - gfx.pixh) / 2 + Fl::box_dy(box());


		_termSize(gfx.ncols, gfx.nrows);

		printf("terminal resized to: %d, %d\n", gfx.ncols, gfx.nrows);
	}
}

/************************************************************************/
#ifndef NDEBUG
void Fl_Term::debug(FILE* where) {
	teDebug(where);
}
#endif

/************************************************************************/
void Fl_Term::draw(void)
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

void Fl_Term::fe_draw_text(int xpos, int ypos, const symbol_t* symbols, int len) {
	const int xo = x() + gfx.xoff;
	const int yo = y() + gfx.yoff;

	if (len > 10) {
		//printf("DrawText(): %d, %d (%d))\n", xpos, ypos, len);
	}

	// Now prepare to draw the actual terminal text
	fl_font(FL_COURIER, def_fnt_size);

	int xp = xo + xpos*font.pixw;
	const int yp = yo + ypos*font.pixh;

	char str[4];
	str[1] = '\0';

	for (int i = 0; i < len; i++) {
		const symbol_t sym = symbols[i];
		const int cp = symbol_get_codepoint(sym);
		const symbol_attributes_t attrs = symbol_get_attributes(sym);

		const symbol_color_t fg_color = symbol_get_fg(sym);
		const symbol_color_t bg_color = symbol_get_bg(sym);

		assert (fg_color >= 0 && fg_color <= 7);
		assert (bg_color >= 0 && bg_color <= 7);

		Fl_Color fg = col_table[fg_color];
		Fl_Color bg = col_table[bg_color];

		if (attrs & SYMBOL_INVERSE) {
			Fl_Color tmp = fg;
			fg = bg;
			bg = tmp;
		}

		fl_color(bg);
		fl_rectf(xp, yp, font.pixw, font.pixh);

		if ((attrs & (SYMBOL_BOLD|SYMBOL_BLINK)) == (SYMBOL_BOLD|SYMBOL_BLINK)) {
			fl_font(FL_COURIER_BOLD_ITALIC, (def_fnt_size));
		} else if (attrs & SYMBOL_BOLD) {
			fl_font(FL_COURIER_BOLD, (def_fnt_size));
		} else if (attrs & SYMBOL_BLINK) {
			fl_font(FL_COURIER_ITALIC, (def_fnt_size));
		} else {
			fl_font(FL_COURIER, def_fnt_size);
		}

/* #ifdef __APPLE__
		char tmp[2] = {cp, '\0'};
		str[0] = fl_latin1_to_local(tmp)[0];
		if (str[0] != tmp[0]) {
			printf("converted from 0x%02x to 0x%02x\n", tmp[0], str[0]);
		}
#else */
//		str[0] = cp;
//#endif
		str[0] = _cpToFltk(cp);
		if (str[0] == 0) {
			str[0] = '?';
		}

		fl_color(fg);
		fl_draw(str, 1, xp, yp+font.pixh-font.descent);

		if (attrs & SYMBOL_UNDERLINE) {
			fl_line(xp, yp, (xp+font.pixw), font.pixh);
		}

		xp += font.pixw;
	}
}

void Fl_Term::fe_draw_clear(int xpos, int ypos, const symbol_color_t bg_color, int len) {
	//	printf("DrawClear: %d, %d (%d))\n", xpos, ypos, len);

		const int xo = x() + gfx.xoff;
		const int yo = y() + gfx.yoff;

		// Now prepare to draw the actual terminal text
		fl_font(FL_COURIER, def_fnt_size);

		int xp = xo + xpos*font.pixw;
		const int yp = yo + ypos*font.pixh;

		char str[4];
		str[1] = '\0';

		for (int i = 0; i < len; i++) {
			assert (bg_color >= 0 && bg_color <= 7);
			Fl_Color bg = col_table[bg_color];

			fl_color(bg);
			fl_rectf(xp, yp, font.pixw, font.pixh);

			xp += font.pixw;
		}
}

void Fl_Term::fe_draw_cursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int xpos, int ypos, int32_t cp) {
	const int xp = x() + gfx.xoff + font.pixw * xpos;
	const int yp = y() + gfx.yoff + font.pixh * ypos;

	assert (fg_color >= 0 && fg_color <= 7);
	const Fl_Color fg = col_table[fg_color];

	// now draw a simple box cursor
	fl_color(fg);
	fl_rectf(xp, yp, font.pixw, font.pixh);
}

void Fl_Term::fe_draw_move(int y, int height, int byoffset) {
	// TODO: implement
}

void Fl_Term::fe_updated() {
	const uint64_t now = getCurrentTime_us();
	if (now-last_draw < _DEFER_DRAWING_US) {
		Fl::remove_timeout(&_s_deferred_update_cb, this);
		Fl::add_timeout(_DEFERRED_DRAWING_DELAY, &_s_deferred_update_cb, this);
		return;
	}
	damage(FL_DAMAGE_USER1);
}

void Fl_Term::fe_reset() {
	// TODO: implement
}

void Fl_Term::fe_bell() {
	fl_beep();
}

void Fl_Term::fe_mouse(int x, int y) {
	// TODO: implement
}

void Fl_Term::fe_title(const int32_t* text) {
	// TODO: implement
}

void Fl_Term::fe_send_back(const int32_t* data) {
	_sendBack(data);
}

void Fl_Term::fe_request_resize(int width, int height) {
	// TODO: almost working, needs to be able resize parent widget as well...
	printf("requested resized to: %d, %d\n", width, height);

	int pixwidth = (width*font.pixw)+Fl::box_dw(box());
	int pixheight = (height*font.pixh)+Fl::box_dh(box());

	resize(x(), y(), pixwidth, pixheight);
}

void Fl_Term::fe_position(int offset, int size) {
	_scrollPosition(offset, size);
}

