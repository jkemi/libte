#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <errno.h>

#include "flkeys.h"

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

	// Create the GTerm
	// determine how big the terminal widget actually is, and create a GTerm to fit
	gt = new gterm_if(this, tw, th);

	// TODO: something weird here:
	// FLTK-1.1 _should_ really return iso8859-1 in event_text() but gives us UTF8 instead
	// also we don't want UCS-4LE on a big-endian machine...
	_fltk_to_cp = iconv_open("UCS-4LE", "UTF8");
	if (_fltk_to_cp == (iconv_t)-1) {
		// TODO: handle somehow
		exit(1);
	}
}

Fl_Term::~Fl_Term() {
	delete gt;
	iconv_close(_fltk_to_cp);
}

// TODO: remove this when unused...
static void _dump(const char* label, const char* text, size_t len) {
	printf("%s", label);
	for (unsigned int i = 0; i < len; i++) {
		printf("0x%02x ", text[i]);
	}
	printf("\n");
}

int32_t Fl_Term::_fltkToCP(const char* text, size_t len) {
	_dump("in: ", text, len);

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
		if(gt->GetMode() & GTerm::NEWLINE) {
			return "\r\n";	// send CRLF if GTerm::NEWLINE is set
		} else {
			return "\r";	// ^M (CR)
		}

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
			gt->SendBack(str);
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
	if ((tw != gt->Width()) || (th != gt->Height()))
	{
		// Then tell the GTerm the new character sizes sizes...
		gt->ResizeTerminal(tw, th);
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
	this->gt->RequestRedraw(0, 0, 80, 50, true);

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

void Fl_Term::DrawStyledText(int xpos, int ypos, int len, const symbol_t* symbols) {
	const int xo = x() + Fl::box_dx(this->box());
	const int yo = y() + Fl::box_dy(this->box());

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
void Fl_Term::ClearChars(int bg_color, int x, int y, int w, int h)
{
	redraw();
}
/************************************************************************/
void Fl_Term::MoveChars(int sx, int sy, int dx, int dy, int w, int h)
{
	redraw();
}

/************************************************************************/
void Fl_Term::DrawCursor(int fg_color, int bg_color, int flags,
                int xpos, int ypos, unsigned char c)
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
// This implements the GTerm interface methods
/************************************************************************/

void gterm_if::UpdateNotification(void) {
	termBox->TerminalUpdated();
}

void gterm_if::DrawText(int fg_color, int bg_color, int flags,
                int x, int y, int len, const uint32_t* string)
{
	termBox->DrawText(fg_color, bg_color, flags, x, y, len, string);
}

/************************************************************************/
void gterm_if::DrawCursor(int fg_color, int bg_color, int flags,
                int x, int y, unsigned char c)
{
	termBox->DrawCursor(fg_color, bg_color, flags, x, y, c);
}

/************************************************************************/

void gterm_if::DrawStyledText(int x, int y, int len, const symbol_t* symbols) {
	termBox->DrawStyledText(x, y, len, symbols);
}

/************************************************************************/
void gterm_if::MoveChars(int sx, int sy, int dx, int dy, int w, int h)
{
//printf("M: (%d, %d): (%d, %d) : (%d, %d)\n", sx, sy, dx, dy, w, h);
	termBox->MoveChars(sx, sy, dx, dy, w, h);
}

/************************************************************************/
void gterm_if::ClearChars(int bg_color, int x, int y, int w, int h)
{
//printf("C: (%d, %d): (%d, %d)\n", x, y, w, h);
	termBox->ClearChars(bg_color, x, y, w, h);
}

/************************************************************************/
void gterm_if::SendBack(const char *data)
{
	if (write_fd) write(write_fd, data, strlen(data));
}

/************************************************************************/
void gterm_if::Bell()
{
	fl_beep();
}

/************************************************************************/
void gterm_if::RequestSizeChange(int w, int h)
{
// Not implemented yet... This should really respond to the VT100 80/132 char
// width commands...
// Also, we might want a different (but similar) method to cope with the
// enclosing Fl_Term being resized...
//	if (w != Width() || h != Height())
//	{
//		?? try and resize the enclosing Fl_Term box/window ??
//		?? or switch to a different font size in the same box? Which is what a VT100 does!

		// Then tell the GTerm the new character sizes sizes...
//		ResizeTerminal(w, h);
//	}
}
/************************************************************************/

/* End of File */
