#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <errno.h>
#include <alloca.h>

#include "strutil.h"

#include "Fl_Term.h"

static void _impl_draw_text (void* priv, int x, int y, const symbol_t* data, int len) {
	((Fl_Term*)priv)->_fe_DrawText(x, y, data, len);
}
static void _impl_draw_clear (void* priv, int x, int y, const symbol_color_t bg_color, int len) {
	((Fl_Term*)priv)->_fe_DrawClear(bg_color, x, y, len);
}
static void _impl_draw_cursor (void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) {
	((Fl_Term*)priv)->_fe_DrawCursor(fg_color, bg_color, attrs, x, y, cp);
}
static void _impl_draw_move (void* priv, int y, int height, int byoffset) {
	((Fl_Term*)priv)->_fe_DrawMove(y, height, byoffset);
}
static void _impl_updated (void* priv) {
	((Fl_Term*)priv)->_fe_Updated();
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
		&_impl_draw_text,
		&_impl_draw_clear,
		&_impl_draw_cursor,
		&_impl_draw_move,
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
	box(FL_THIN_DOWN_FRAME);

	def_fnt_size = sz;
	fl_font(FL_COURIER, def_fnt_size);
	fh = fl_height();
	cw = fl_width("MHW#i1l") / 7; // get an average char width, in case of Prop Fonts!
	fw = (int)(cw + 0.5);
	fnt_desc = fl_descent();

	tw = (int)((w() - Fl::box_dw(box())) / cw);
	th = (h() - Fl::box_dh(box())) / fh;

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
		return (te_handle_button(_te, tekey) != 0);
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

		printf("cp was: %d\n", cp);

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
			te_handle_keypress(_te, cp, (te_modifier_t)mod);
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
	Fl_Box::resize( x, y, W, H);

	tw = (int)((w() - Fl::box_dw(box())) / cw);
	th = (h() - Fl::box_dh(box())) / fh;

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

	te_reqest_redraw(_te, 0, 0, tw, th, false);

	// restore the clipping rectangle...
	fl_pop_clip();
} /* end of draw() method */


//////////////////////////////////////////////////////////////////////
// TE_Frontend methods
//////////////////////////////////////////////////////////////////////

void Fl_Term::_fe_Updated(void) {
	redraw();
}

void Fl_Term::_fe_DrawText(int xpos, int ypos, const symbol_t* symbols, int len) {
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

		Fl_Color fg = col_table[fg_color];
		Fl_Color bg = col_table[bg_color];

		if (attrs & SYMBOL_INVERSE) {
			Fl_Color tmp = fg;
			fg = bg;
			bg = tmp;
		}

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

void Fl_Term::_fe_DrawClear(symbol_color_t bg_color, int xpos, int ypos, int len)
{
	printf("ClearChars: %d, %d (%d))\n", xpos, ypos, len);

	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());

	// Now prepare to draw the actual terminal text
	fl_font(FL_COURIER, def_fnt_size);

	int xp = xo + xpos*fw;
	const int yp = yo + ypos*fh;

	char str[4];
	str[1] = '\0';

	for (int i = 0; i < len; i++) {
		Fl_Color bg = col_table[bg_color];

		fl_color(bg);
		fl_rectf(xp, yp, fw, fh);

		xp += fw;
	}
}

void Fl_Term::_fe_DrawMove(int y, int height, int byoffset)
{
	//redraw();
}


void Fl_Term::_fe_DrawCursor(symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs,
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
