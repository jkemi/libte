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
#include "pty/term.h"


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


static void term_size_cb(int width, int height, void* priv)  {
	term_set_window_size(pty_fd, width, height);
}

/**
 * Called whenever input is received from pty process.
 */
static void mfd_cb(int mfd, void* unused_priv)
{
	ssize_t ret = read(mfd, buf+buffill, (BUFSIZE-buffill)*sizeof(unsigned char));
	if (ret == -1) {
		// TODO: hello
		exit(0);
		//return;
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
//		str_cps_hexdump("from pty: ", cpbuf, cpcount);

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
	term->box(FL_DOWN_FRAME);

	main_win->end();

	main_win->resizable(term);


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
	term->setTermSizeCB(&term_size_cb, NULL);

	// show the windows
	main_win->show(argc, argv);

	int exit_res = Fl::run();

	pty_restore();

	return exit_res;
}
