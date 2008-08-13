#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <errno.h>
#include <alloca.h>

#include "strutil.h"

#include "flkeys.h"

#include "Fl_Term.h"

static void _impl_draw (void* priv, int x, int y, const symbol_t* data, int len) {
	((Fl_Term*)priv)->DrawStyledText(x, y, data, len);
}
static void _impl_clear (void* priv, int x, int y, const symbol_color_t bg_color, int len) {
	((Fl_Term*)priv)->ClearChars(bg_color, x, y, len);
}
static void _impl_draw_cursor (void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) {
	((Fl_Term*)priv)->DrawCursor(fg_color, bg_color, attrs, x, y, cp);
}
static void _impl_scroll (void* priv, int y, int height, int offset) {
	((Fl_Term*)priv)->Scroll(y, height, offset);
}
static void _impl_resized (void* priv, int width, int height) {
	//TODO: implement me
}
static void _impl_updated (void* priv) {
	((Fl_Term*)priv)->TerminalUpdated();
}
static void _impl_reset (void* priv) {
	//TODO: implement me
}
static void _impl_bell (void* priv) {
	fl_beep();
}
static void _impl_mouse (void* priv, int x, int y) {
	//TODO: implement me
}
static void _impl_title (void* priv, const int32_t* text) {
	//TODO: implement me
}
static void _impl_send_back (void* priv, const int32_t* data) {
	((Fl_Term*)priv)->_sendBack(data);
}
static void _impl_request_resize (void* priv, int width, int height) {
	//TODO: implement me
}

const static TE_Frontend _impl_callbacks = {
		&_impl_draw,
		&_impl_clear,
		&_impl_draw_cursor,
		&_impl_scroll,
		&_impl_resized,
		&_impl_updated,
		&_impl_reset,
		&_impl_bell,
		&_impl_mouse,
		&_impl_title,
		&_impl_send_back,
		&_impl_request_resize,
};

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


/************************************************************************/
// This is the implementation of the user-facing parts of the widget...
/************************************************************************/
Fl_Term::Fl_Term(int sz, int X, int Y, int W, int H, const char *L) : Fl_Box(X,Y,W,H,L)
{
	def_fnt_size = sz;
	fl_font(FL_COURIER, def_fnt_size);
	fh = fl_height();
	cw = fl_width("MHW#i1l") / 7; // get an average char width, in case of Prop Fonts!
	fw = (int)(cw + 0.5);
	fnt_desc = fl_descent();

	tw = (int)((w() - 4) / cw);
	th = (h() - 4) / fh;

	crs_x = crs_y = 1;
	crs_fg = 7; // white
	crs_bg = 0; // black
	crs_flags = 0;

	// determine how big the terminal widget actually is, and create a GTerm to fit
	_te = te_create(&_impl_callbacks, this, tw, th);

	// TODO: something weird here:
	// FLTK-1.1 _should_ really return iso8859-1 in event_text() but gives us UTF8 instead
	// also we don't want UCS-4LE on a big-endian machine...
	_fltk_to_cp = iconv_open("UCS-4LE", "UTF8");
	if (_fltk_to_cp == (iconv_t)-1) {
		// TODO: handle somehow
		exit(1);
	}

	_send_back_func = 0;
}

Fl_Term::~Fl_Term() {
	te_destroy(_te);
	iconv_close(_fltk_to_cp);
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

	printf ("result: %d %p, %p: %d\n", (int)result, &cp, outbuf, (int)written);

	if (written > 0) {
		return cp;
	} else {
		return -1;
	}
}

const char* Fl_Term::_handle_keyevent(void) {
	const int keysym = Fl::event_key();

	switch (keysym) {
	case FL_BackSpace:
		return "\010";		// ^H

	case FL_Enter:
		te_handle_button(_te, TE_KEY_RETURN);

	case FL_Escape:
		return "\033";		// ESC

	case FL_Tab:
		return "\t";		// ^I (tab)

	}
	// OK, still not done - lets try looking up the VT100 key mapping tables...
	// Also - how to handle the "windows" key on PC style kbds?
	const keyseq* tables[] = {keypadkeys, cursorkeys, otherkeys, NULL};

	for (const keyseq** table = tables; *table != NULL; table++) {
		const char* str = find_key(keysym, *table);
		if (str) {
			return str;
		}
	}

	if (Fl::event_length() > 0) {

		// TODO: this magic should probably be MB_CUR_MAX+1 instead..
		const size_t bufsz = 8;

		static char buf[bufsz];
		memset(buf, 0, sizeof(char)*bufsz);

		/*
		int ndeleted;
		if (Fl::compose(ndeleted) == false) {
			printf("compose() == false, %d\n", ndeleted);
			return NULL;
		}
		*/

		char* dest;
		if (Fl::event_alt()) {
			buf[0] = '\033';
			dest = buf+1;
		} else {
			dest = buf;
		}

		const int32_t cp = _fltkToCP(Fl::event_text(), Fl::event_length());
		printf("cp was: %d\n", cp);
		if (cp >= 0) {
			mbstate_t ps = {0};
			const size_t mblen = wcrtomb(dest, cp, &ps);
			if (mblen > 0) {
				dest[mblen] = '\0';
				return buf;
			}
		}
	}

	return NULL;
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
		gt->SendBack(txt);
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

// TODO: remove this method
void Fl_Term::_sendBackMBS(const char* data) {
	const size_t len = str_mbslen(data);
	int32_t* buf = (int32_t*)alloca(sizeof(int32_t*)*(len+1));
	int res = str_mbs_to_cps_n(buf, data, len+1, strlen(data), NULL, NULL);
	assert (res == 0);

	_sendBack(buf);
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
		const char* str = _handle_keyevent();
		if (str != NULL) {
			_sendBackMBS(str);
			return 1;
		} else {
			return 0;
		}
		break;
	}
	default:
		return 0;
	}
}

/************************************************************************/
// handle window resizing - THIS DOES NOT WORK RIGHT!
void Fl_Term::resize(int x, int y, int W, int H)
{
	Fl_Box::resize( x, y, W, H);

	tw = (int)((w() - 4) / cw);
	th = (h() - 4) / fh;

	if (tw != te_get_width(_te) || th != te_get_height(_te)) {
		// Then tell the GTerm the new character sizes sizes...
		te_resize(_te, tw, th);
	}
}

/************************************************************************/
void Fl_Term::draw(void)
{
	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());
	const int wd = w() - Fl::box_dw(this->box());
	const int ht = h() - Fl::box_dh(this->box());

	Fl_Box::draw();
	fl_push_clip(xo, yo, wd, ht);

	fl_color(FL_BLACK);
	fl_rectf(xo, yo, wd, ht);

	// TODO: fix nrows ncols etc here
	te_reqest_redraw(_te, 0, 0, 80, 50, true);

	// restore the clipping rectangle...
	fl_pop_clip();
} /* end of draw() method */

/************************************************************************/


void Fl_Term::TerminalUpdated(void) {
	redraw();
}

void Fl_Term::DrawText(int fg_color, int bg_color, int flags,
                      int x, int y, int len, const uint32_t* string)
{
	redraw();
} // DrawText

void Fl_Term::DrawStyledText(int xpos, int ypos, const symbol_t* symbols, int len) {
	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());

	printf("DrawStyledText(): %d, %d (%d))\n", xpos, ypos, len);

	// Now prepare to draw the actual terminal text
	fl_font(FL_COURIER, def_fnt_size);

	int xp = xo + xpos*fw;
	const int yp = yo + ypos*fh;

	char str[4];
	str[1] = '\0';

	for (int i = 0; i < len; i++) {
		const symbol_t sym = symbols[i];
		const int cp = symbol_get_codepoint(sym);
		const symbol_attributes_t attrs = symbol_get_attributes(sym);

		const symbol_color_t fg_color = symbol_get_fg(sym);
		const symbol_color_t bg_color = symbol_get_bg(sym);

		const Fl_Color fg = col_table[fg_color];
		const Fl_Color bg = col_table[bg_color];

		fl_color(bg);
		fl_rectf(xp, yp, fw, fh);

		if ((attrs & (SYMBOL_BOLD|SYMBOL_BLINK)) == (SYMBOL_BOLD|SYMBOL_BLINK)) {
			fl_font(FL_COURIER_BOLD_ITALIC, (def_fnt_size));
		} else if (attrs & SYMBOL_BOLD) {
			fl_font(FL_COURIER_BOLD, (def_fnt_size));
		} else if (attrs & SYMBOL_BLINK) {
			fl_font(FL_COURIER_ITALIC, (def_fnt_size));
		} else {
			fl_font(FL_COURIER, def_fnt_size);
		}

		str[0] = cp;
		fl_color(fg);
		fl_draw(str, 1, xp, yp+fh-fnt_desc);

		if (attrs & SYMBOL_UNDERLINE) {
			fl_line(xp, yp, (xp+fw), fh);
		}

		xp += fw;
	}
}

/************************************************************************/
void Fl_Term::ClearChars(symbol_color_t bg_color, int x, int y, int len)
{
	redraw();
}
/************************************************************************/
void Fl_Term::Scroll(int y, int height, int offset)
{
	redraw();
}

/************************************************************************/
void Fl_Term::DrawCursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs,
                int xpos, int ypos, int32_t cp)
{
	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());

	const int xp = (int)( xo + (float)cw * xpos);
	const int yp = (int)( yo + (float)fh * ypos);

	const int w = fw;
	const int h = fh;

	const Fl_Color fg = col_table[fg_color];
	const Fl_Color bg = col_table[bg_color];


	fl_color(bg);
	fl_rectf(xp, yp, w, h);

	// now draw a simple box cursor
	fl_color(fg);
	fl_rectf(xp, yp, w, h);
}

/************************************************************************/

/* End of File */
