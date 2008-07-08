
#include "Fl_Term.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Fl_Term.h"

#include "flkeys.h"

#define TXT_QUEUE	64
#define TQ_MASK     (TXT_QUEUE-1)

struct stval {
	char fg;
	char bg;
	int flags;
};

/* VT100 colour table - map Colors to FL-colors:
0 - black
1 - red
2 - green
3 - yellow
4 - blue
5 - magenta
6 - cyan
7 - white
*/
static int col_table[] =
{	FL_BLACK,
	FL_RED,
	FL_GREEN,
	FL_YELLOW,
	FL_BLUE,
	FL_DARK_MAGENTA,
	FL_CYAN,
	FL_WHITE,
	FL_DARK_BLUE,
	FL_DARK_CYAN,
	FL_DARK_RED,
	FL_DARK_YELLOW,
	FL_DARK_GREEN,
	FL_MAGENTA };


/************************************************************************/
// This is the implementaton of the user-facing parts of the widget...
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
	printf("TS: %d %d\n", tw, th);
	gt = new gterm_if(this, tw, th);

} // Fl_Term constructor

/************************************************************************/
// handle keyboard focus etc
int Fl_Term::handle(int ev)
{
	int key, el;
	char key_c = 0;
	const char *str = NULL;
	char txt[2]; txt[1] = 0;
	switch (ev)
	{
	case FL_FOCUS:
	case FL_UNFOCUS:
	return 1;

	case FL_KEYBOARD:
		el = Fl::event_length();
		key = Fl::event_key();

		if(el) key_c = Fl::event_text()[0]; // probably a dodgy assumption...
		if((key >= ' ') && (key <= '~')) // simple ASCII
		{
			if ((key >= 'a') && (key <= 'z'))
			{
				if (Fl::event_ctrl()) // ctrl-key
					key_c = key - 0x60;
			// Also need to do something about ALT and META at this point...
			}
			txt[0] = key_c & 0x7F;
			gt->SendBack(txt);
			return 1;
		}
		// Hmm, if we get here, it's probably not a simple printable key...
		switch (key)
		{
		case FL_BackSpace:
			gt->SendBack("\010"); // ^H
			return 1;

		case FL_Enter:
			if(gt->GetMode() & GTerm::NEWLINE)
				gt->SendBack("\r\n"); // send CRLF if GTerm::NEWLINE is set
			else
				gt->SendBack("\r"); // ^M (CR)
			return 1;

		case FL_Escape:
			gt->SendBack("\033"); // ESC
			return 1;

		case FL_Tab:
			gt->SendBack("\t"); // ^I (tab)
			return 1;

		// OK, still not done - lets try looking up the VT100 key mapping tables...
		// Also - how to handle the "windows" key on PC style kbds?
		default:
			str = find_key(key, keypadkeys);
			if (!str) {
				str = find_key(key, cursorkeys);
			}
			if (!str) {
				str = find_key(key, otherkeys);
			}
			if (str) {
				gt->SendBack(str);
			}
			break;
		} // end of "key" switch
		return 1; // return 1 for FL_KEYBOARD even if we don't know what they key is for!

	default: // Any event we don't care about
		return 0;
	}// end of event switch
	return 0; // should never get here anyway...
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
	// TODO: this border width/height hack must die!
	int wd = w() - 4;
	int ht = h() - 4;

	int xo = x() + 2;
	int yo = y() + 2;

	Fl_Box::draw();
	fl_push_clip(xo, yo, wd, ht);

	fl_color(FL_BLACK);
	fl_rectf(xo, yo, wd, ht);

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
	// TODO: this border width/height hack must die!
	int wd = w() - 4;
	int ht = h() - 4;

	int xo = x() + 2;
	int yo = y() + 2;


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

		const int fg = col_table[fg_color];
		const int bg = col_table[bg_color];

		fl_color(bg);
		fl_rectf(xp, yp, fw, fh);

		if (attrs & (SYMBOL_BOLD|SYMBOL_BLINK) == (SYMBOL_BOLD|SYMBOL_BLINK)) {
			fl_font(FL_COURIER_BOLD_ITALIC, (def_fnt_size+2));
		} else if (attrs & SYMBOL_BOLD) {
			fl_font(FL_COURIER_BOLD, (def_fnt_size+2));
		} else if (attrs & SYMBOL_BLINK) {
			fl_font(FL_COURIER_ITALIC, (def_fnt_size+2));
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
	// TODO: this border width/height hack must die!
	int wd = w() - 4;
	int ht = h() - 4;

	int xo = x() + 2;
	int yo = y() + 2;

	const int xp = (int)( xo + (float)cw * xpos);
	const int yp = (int)( yo + (float)fh * ypos);

	// TODO: use fl_width() or fl_descent() or something at least..
	const int w = 10;
	const int h = 16;

	const int fg = col_table[fg_color];
	const int bg = col_table[bg_color];


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
