#ifndef HAVE_FL_TERM_HDR
#define HAVE_FL_TERM_HDR

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <Fl/fl_draw.H>
#include <FL/fl_ask.H>

#include "gterm/gterm.hpp"

class Fl_Term;

/************************************************************************/
// This class instantiates the GTerm interface and the implementations
// of the virtual methods from that base class
class gterm_if : public GTerm
{
protected:
	Fl_Term *termBox; // points to parent Fl_Term instance
	int write_fd;

public:
	// constructor
	gterm_if(Fl_Term *t, int w, int h) : GTerm(w, h) {termBox = t; write_fd = 0;}

	// Implement methods needed by GTerm
	void DrawText(int fg_color, int bg_color, int flags, int x, int y, int len, unsigned char *string);
	void DrawCursor(int fg_color, int bg_color, int flags, int x, int y, unsigned char c);
	void MoveChars(int sx, int sy, int dx, int dy, int w, int h);
	void ClearChars(int bg_color, int x, int y, int w, int h);
	void SendBack(const char *data);
	void Bell();
	void RequestSizeChange(int w, int h); // NOT WORKING YET

	// Implement some helpers
	void set_write_fd(int fd) {write_fd = fd;}
};

/************************************************************************/
// The text_box class is only used for diagnostic purposes...
class text_box : public Fl_Box
{
protected:
	void draw(void);

public:
     text_box(int X, int Y, int W, int H, const char *L=0) : Fl_Box(X,Y,W,H,L) { }
};
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
	text_box *listall;
	gterm_if *gt;

public:
	// constrctor
	Fl_Term(int fs, int X, int Y, int W, int H, const char *L=0);

	// handle keyboard etc.
	int handle(int);

	// handle window resizing (NOT WORKING YET)
	void resize(int x, int y, int w, int h);

	// basic text drawing
	void DrawText(int fg_color, int bg_color, int flags, int x, int y, int len, unsigned char *string);
	void ClearChars(int bg_color, int x, int y, int w, int h);
	void MoveChars(int sx, int sy, int dx, int dy, int w, int h);
	void DrawCursor(int fg_color, int bg_color, int flags, int x, int y, unsigned char c);

	// text box view width/height
	int get_w(void) {return tw;}
	int get_h(void) {return th;}

	// get a pointer to the enclosed GTerm
	gterm_if *get_gterm(void) {return gt;}

	// font handling
	void font_size(int sz) {def_fnt_size = sz;}

	// diagnostic view...
	void set_listview(text_box *l) {listall = l;}
};
/************************************************************************/


#endif // HAVE_FL_TERM_HDR

/* End of File */
