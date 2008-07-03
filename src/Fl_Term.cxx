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

// These ought to be class private, but are in global scope to allow the diagnostic window
// to process them...
static char  lines[TXT_QUEUE][GT_MAXWIDTH]; // ASCII text
static stval style[TXT_QUEUE][GT_MAXWIDTH]; // char colour buffer
static int topvis = 0;

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
// The text_box class is only used for diagnostic purposes...
/************************************************************************/
void text_box::draw(void)
{
	int line = 0;
	int wd = w() - 4;
	int ht = h() - 4;

	int xo = x() + 2;
	int yo = y() + 2;

	Fl_Box::draw();
	fl_push_clip(xo, yo, wd, ht);

	fl_color(FL_BLACK);
	fl_rectf(xo, yo, wd, ht);

	fl_font(FL_COURIER, 10);
	int fh = fl_height();
	float cw = fl_width("MHW#i1l") / 7; // get an average char width, in case of Prop Fonts!
	int fw = (int)cw;
	int font_swap = 0;
	int lx = xo + 2;
	int ly = yo + fh;

	char str[4]; str[1] = 0;
	while ((line < TXT_QUEUE) && (ly < ht))
	{
		int pos = 0;
		int px = lx + (int)(2.0*cw);
		float fpos = (float)px;
		if (line == topvis)
			str[0] = '>';
		else
			str[0] = ' ';
		fl_color(FL_RED);
		fl_draw(str, lx, ly);
		while((lines[line][pos] != 0) && (pos < GT_MAXWIDTH))
		{
			// draw cell background
			fl_color(FL_BLACK);
			int cbg = col_table[(int)style[line][pos].bg];
			if (cbg != FL_BLACK)
			{
				fl_color(cbg);
				fl_rectf(px, (ly - fh + 3), fw, fh);
			}

			// check font setting
			if(style[line][pos].flags & GTerm::BOLD)
			{
				font_swap = -1;
				fl_font(FL_COURIER_BOLD, (12));
			}

			// read cell char, set colour and draw
			char cc = lines[line][pos];
			str[0] = cc;
			fl_color(col_table[(int)style[line][pos].fg]);
			fl_draw(str, px, ly);

			// check if underline is needed
			if(style[line][pos].flags & GTerm::UNDERLINE)
			{
				fl_line(px, ly, (px+fw), ly);
			}

			// restore font setting
			if(font_swap)
			{
				font_swap = 0;
				fl_font(FL_COURIER, 10);
			}

			fpos = fpos + cw;
			px = (int)fpos;
			pos = pos + 1;
			if (px >= wd)
				break;
		}
		line = line + 1;
		ly = ly + fh;
	}

	fl_pop_clip();
} /* end of draw() method */

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

	// flush the terminal layout buffers
	memset(lines, 0x20, sizeof(lines));
	memset(style, 0, sizeof(style));

	// Create the GTerm
	// determine how big the terminal widget actually is, and create a GTerm to fit
	printf("TS: %d %d\n", tw, th);
	gt = new gterm_if(this, tw, th);

	// Ensure the diagnostic view is disabled by default
	listall = NULL;

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
	int line = topvis;
	int wd = w() - 4;
	int ht = h() - 4;

	int xo = x() + 2;
	int yo = y() + 2;

	int font_swap = 0;

	Fl_Box::draw();
	fl_push_clip(xo, yo, wd, ht);

	fl_color(FL_BLACK);
	fl_rectf(xo, yo, wd, ht);
	int lx = xo + 2;
	int ln = 0, ly, pos, cbg;
	float fpos;
	int gt_mode = gt->GetMode();

	// If the cursor is visible (check for GTerm::CURSORINVISIBLE here)
	if((gt_mode & GTerm::CURSORINVISIBLE) == 0)
	{ // then we should draw it.
		// First get its screen co-ords
		fpos = (float)lx + cw * (float)crs_x;
		pos = (int)fpos;
		ly = yo + (crs_y * fh) + fnt_desc;
		// now draw any cursor background rectangle...
		cbg = col_table[crs_bg];
		if (cbg != FL_BLACK)
		{
			fl_color(cbg);
			fl_rectf(pos, ly, fw, fh);
		}
		// now draw a simple box cursor
		fl_color(col_table[crs_fg]);
		fl_rect(pos, ly, fw, fh);
	}

	// Now prepare to draw the actual terminal text
	fl_font(FL_COURIER, def_fnt_size);
	lx = xo + 2;
	ly = yo + fh;

	char str[4]; str[1] = 0;
//	while ((ly < ht) && (line < TXT_QUEUE))
	while ((ln < th) && (line < TXT_QUEUE))
	{
		ln = ln + 1;
		pos = 0;
		fpos = (float)lx;
		int px = lx;
		while((lines[line][pos] != 0) && (pos < GT_MAXWIDTH))
		{
			// draw cell background
			fl_color(FL_BLACK);
			cbg = col_table[(int)style[line][pos].bg];
			if (cbg != FL_BLACK)
			{
				fl_color(cbg);
				fl_rectf(px, (ly - fh + fnt_desc), fw, fh);
			}

			// check font setting - first, check for BOLD text
			if(style[line][pos].flags & GTerm::BOLD)
			{
				font_swap = -1;
				fl_font(FL_COURIER_BOLD, (def_fnt_size+2));
			}
			// represent BLINK by using italic text
			if(style[line][pos].flags & GTerm::BLINK)
			{
				if (font_swap) // already set BOLD flag
				{
					fl_font(FL_COURIER_BOLD_ITALIC, (def_fnt_size+2));
					font_swap = -3;
				}
				else
				{
					fl_font(FL_COURIER_ITALIC, def_fnt_size);
					font_swap = -2;
				}
			}

			// read cell char, set colour and draw
			char cc = lines[line][pos];
			str[0] = cc;
			fl_color(col_table[(int)style[line][pos].fg]);
			fl_draw(str, px, ly);

			// check if underline is needed
			if(style[line][pos].flags & GTerm::UNDERLINE)
			{
				fl_line(px, ly, (px+fw), ly);
			}

			// restore font setting
			if(font_swap)
			{
				font_swap = 0;
				fl_font(FL_COURIER, def_fnt_size);
			}

			// index to next char cell position
			fpos = fpos + cw;
			px = (int)fpos;
			pos = pos + 1;
			// have we reached the edge?
			if (px >= wd)
				break;
		}
		// next text line from buffer
		line = (line + 1) & TQ_MASK;
		// next line position
		ly = ly + fh;
	}

	// restore the clipping rectangle...
	fl_pop_clip();

	if(listall) listall->redraw(); // diagnostic window

} /* end of draw() method */

/************************************************************************/
void Fl_Term::DrawText(int fg_color, int bg_color, int flags,
                      int x, int y, int len, unsigned char *string)
{
	if(fg_color == bg_color)
	{
		bg_color = (bg_color - 1) & 7;
		flags |= GTerm::BOLD;
	}

	if ((fg_color == 0) && (flags & GTerm::BOLD))
	{
		fg_color = 7;
		flags &= ~GTerm::BOLD;
	}

	if (flags & GTerm::INVERSE)
	{
		int t = fg_color;
		fg_color = bg_color;
		bg_color = t;
	}

	// find line in TQ buffer
	int lineIdx = topvis + y;
	lineIdx = lineIdx & TQ_MASK;

	// limit line length
	int last = len + x;
	if(last > GT_MAXWIDTH)
	{
		last = GT_MAXWIDTH;
		len = GT_MAXWIDTH - x;
	}

	// copy string chars
	strncpy(&lines[lineIdx][x], (const char *)string, len);

	// set style buffer values
	for(int idx = x; idx < last; idx++)
	{
		style[lineIdx][idx].fg = fg_color;
		style[lineIdx][idx].bg = bg_color;
		style[lineIdx][idx].flags = flags;
	}

	redraw();
} // DrawText

/************************************************************************/
void Fl_Term::ClearChars(int bg_color, int x, int y, int w, int h)
{
	int line = (topvis + y) & TQ_MASK;
	int lastpos = x + w;
	if (lastpos > GT_MAXWIDTH)
		lastpos = GT_MAXWIDTH;

	for(int li = 0; li < h; li++)
	{
		for(int idx = x; idx < lastpos; idx++)
		{
			lines[line][idx] = ' '; // clear all "empty" chars to "spaces"
			// fill style buffer here too...
			style[line][idx].fg = bg_color;
			style[line][idx].bg = bg_color;
			style[line][idx].flags = 0;
		}
		line = (line + 1) & TQ_MASK;
	}
	redraw();
}
/************************************************************************/
void Fl_Term::MoveChars(int sx, int sy, int dx, int dy, int w, int h)
{
	if ((sx != 0) || (dx != 0))
	{
		fprintf(stderr, "non-zero move alignment %d %d\n", sx, dx);
	}

	int delta = sy - dy;
	topvis = (topvis + delta) & TQ_MASK;
	redraw();
}

/************************************************************************/
void Fl_Term::DrawCursor(int fg_color, int bg_color, int flags,
                int x, int y, unsigned char c)
{
// TBD:: What to draw for the cursor anyway? Just draw a box for now
	crs_x = x; crs_y = y;
	crs_fg = fg_color; 	crs_bg = bg_color;
	crs_flags = flags;
}

/************************************************************************/
// This implements the GTerm interface methods
/************************************************************************/
void gterm_if::DrawText(int fg_color, int bg_color, int flags,
                int x, int y, int len, unsigned char *string)
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
