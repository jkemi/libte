#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <locale.h>
#include <string.h>

#include <stdint.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scrollbar.H>

#include "Flx_Terminal.hpp"

#include "pty/pty.h"


#include "strutil.h"

typedef unsigned int uint;


// Set this to use deferred rather than direct terminal updates
//#define USE_DEFERUPDATE

/************************************************************************/
static Fl_Window* main_win = NULL;
static Flx_Terminal* term = NULL;
static int pty_fd = -1;

#define BUFSIZE		1024
static char			buf[BUFSIZE];
static size_t		buffill = 0;

/************************************************************************/

static void send_back_cb(const int32_t* data, size_t size, void* priv) {
	size_t n = str_cpslen(data);
	size_t nbytes = MB_CUR_MAX*(n+1);
	char tmp[nbytes];

	size_t nwritten;

	str_cps_to_mbs_n(tmp, data, nbytes, n, &nwritten, NULL);

	str_mbs_hexdump("to pty: ", tmp, nwritten);
	write(pty_fd, tmp, nwritten);
}


/**
 * Called whenever input is received from pty process.
 */
static void mfd_cb(int mfd, void* unused_priv)
{
	ssize_t ret = read(mfd, buf+buffill, (BUFSIZE-buffill)*sizeof(unsigned char));
	if (ret == -1) {
		return;
	}

	size_t bytesread = ret;
	str_mbs_hexdump("from pty(mbs): ", buf+buffill, bytesread);

	buffill += bytesread;

	int32_t	cpbuf[1024];
	size_t cpcount;


	if (str_mbs_to_cps_n(cpbuf, buf, 1024, buffill, &cpcount, &bytesread) != 0) {
		//TODO: this happens.. try pilned sedan "å" så skiter det sig nog..
		buffill = 0;
		return;
/*		main_win->hide();
		abort();*/
	} else {
		str_cps_hexdump("from pty: ", cpbuf, cpcount);

		term->fromChild(cpbuf, cpcount);

		const size_t remaining = buffill-bytesread;
		memcpy(buf, buf+bytesread, remaining);
		buffill = remaining;
	}
}

/************************************************************************/
static void upd_term_cb(void *v)
{
//	te_update(termBox->_te);
	Fl::repeat_timeout(0.1, upd_term_cb, v);
}
/************************************************************************/
static void quit_cb(Fl_Button *, void *)
{
	char str[] = "exit\n";
	int count = strlen(str);
	write(pty_fd, str, count);

	usleep(100000); // 100ms
	main_win->hide();
}

/*
int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");
	Fl::args(argc, argv);

	int mfd; // master fd for pty

	// measure the default font, determine how big to make the terminal window
	// This is just a cheat to save working out how big to make the box for a
	// 80x24 terminal window...
	fl_font(FL_COURIER, def_fnt_size);
	int fh = fl_height();
	float cw = fl_width("MHW#i1l") / 7; // get an average char width, in case of Prop Fonts!

	int tw = (int)(cw * 80.5 + 4.0); // 80 chars...
	int th = 24 * fh + 4 + (fh/2);   // by 24 lines...

	// create the main window and the terminal widget
	main_win = new Fl_Double_Window(tw+25, th+60);
	main_win->box(FL_NO_BOX);
	main_win->begin();

	Fl_Group* term_grp = new Fl_Group(5, 5, tw, th+20);
	term_grp->begin();
	term_grp->box(FL_NO_BOX);
	scroll = new Fl_Scrollbar(tw+5, 5, 15, th);
	termBox = new Fl_Term(def_fnt_size, 5, 5, tw, th);
	term_grp->end();

	// create some buttons for controlling the widget
	Fl_Group * but_grp = new Fl_Group(5, th+8, tw+15, 40);
	but_grp->begin();
	but_grp->box(FL_ENGRAVED_BOX);

	Fl_Button *dummy = new Fl_Button(6, th+10, 2, 2);
	dummy->box(FL_NO_BOX);
	dummy->clear_visible_focus();

	Fl_Button *quit = new Fl_Button((tw - 65), th+12, 60, 30);
	quit->label("Quit");
	quit->box(FL_THIN_UP_BOX);
	quit->callback((Fl_Callback *)quit_cb);

	but_grp->end();
	but_grp->resizable(dummy);

	main_win->end();
	main_win->label("Terminal test");
	//Fl::visible_focus(0);

// DO NOT set a resizable on the terminal window - it does not work right yet!
//	main_win->resizable(termBox);
//	main_win->size_range(500, 200, 0, 0, 0, 0, 0);

	// show the windows
	main_win->show(argc, argv);

	// Give the terminal the focus by default
	Fl::focus(termBox);
	Fl::focus(scroll);

	// spawn shell in pseudo terminal
	mfd = pty_spawn("/bin/sh");
	if (mfd < 0) {
		exit(-1);
	}

	pty_fd = mfd;

	// we want non-blocking reads from pty output
	fcntl(mfd, F_SETFL, O_NONBLOCK);

	scroll_cb(NULL, 0, 0);

	// Attach the GTerm terminal i/o to the fd...
	termBox->set_send_back_func(&send_back_cb, NULL);

	termBox->set_scroll_func(&scroll_cb, NULL);

//	//configure terminal for deferred display updates
//#ifdef USE_DEFERUPDATE
//	termIO->set_mode_flag(GTerm::DEFERUPDATE);   // enable deferred update
//	// basic timeout to poll the terminal for refresh - do this if we select DEFERUPDATE
//	Fl::add_timeout(0.3, upd_term_cb, (void *)termIO);
//#else
//	termIO->clear_mode_flag(GTerm::DEFERUPDATE); // disable deferred - direct updates are used
//#endif

	// add the pty to the fltk fd list, so we can catch any output
	Fl::add_fd(mfd, mfd_cb, NULL);

	scroll->callback((Fl_Callback*)&did_scroll);
//	scroll->when(FL_WHEN_CHANGED);

	int exit_res = Fl::run();
	pty_restore();
	return exit_res;
}

*/

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");
	Fl::args(argc, argv);

	const uint W = 654;
	const uint H = 410;

	// create the main window and the terminal widget
	main_win = new Fl_Double_Window(W, H);
	main_win->box(FL_NO_BOX);
	main_win->begin();

	// inner dimensions
	const int iw = W - Fl::box_dw(main_win->box());
	const int ih = H - Fl::box_dh(main_win->box());

	int x = 0 + Fl::box_dx(main_win->box());
	int y = 0 + Fl::box_dy(main_win->box());

	term = new Flx_Terminal(x, y, iw, ih, 0);

	main_win->end();


	// spawn shell in pseudo terminal
	pty_fd = pty_spawn("/bin/bash");
	if (pty_fd < 0) {
		exit(-1);
	}
	// add the pty to the fltk fd list, so we can catch any output
	Fl::add_fd(pty_fd, mfd_cb, NULL);
	// we want non-blocking reads from pty output
	fcntl(pty_fd, F_SETFL, O_NONBLOCK);


	term->setToChildCB(&send_back_cb, NULL);

	// show the windows
	main_win->show(argc, argv);

	int exit_res = Fl::run();

	pty_restore();

	return exit_res;
}
