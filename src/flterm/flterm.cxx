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
#include <FL/fl_ask.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>

#include <libte/libte.h>

#include "pty/pty.h"
#include "pty/term.h"

#include "strutil.h"

#include "Flx_ScrolledTerm.hpp"


static Fl_Window* main_win;

class ResizeHandler : public Flx::IResizableParent {
	void event_size_range(int minw, int minh, int maxw, int maxh, int stepw, int steph) {
		const int dw = Fl::box_dw(main_win->box());
		const int dh = Fl::box_dh(main_win->box());

		minw += dw;
		maxw += dw;
		minh += dh;
		maxh += dh;

		main_win->size_range(minw, minh, maxw, maxh, stepw, steph);
	}

	void event_want_size(int width, int height) {
		const int dw = Fl::box_dw(main_win->box());
		const int dh = Fl::box_dh(main_win->box());

		width += dw;
		height += dh;

		printf("SH:My size is %d,%d, wanted %d,%d\n", main_win->w(), main_win->h(), width, height);

		main_win->size(width, height);
		//main_win->resizable(term);
	}
	void event_title(const int32_t* text, int len) {
		size_t nbytes = 6*(len) + 1;
		char tmp[nbytes];

		char* dest = tmp;
		for (int i=0; i<len; i++) {
			int r = fl_utf8encode(text[i], dest);
			dest += r;
		}
		*dest = '\0';
		
//		printf("changed title to: %s\n", tmp);
		main_win->copy_label(tmp);
	}
};

int main(int argc, char** argv)
{
	printf("libte compile version: %s, linked version: %s\n", TE_HEADER_VERSION, te_binary_version);
	
	// TODO: make configurable? or use "" for default?
	if (setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
		exit(EXIT_FAILURE);
	}
	Fl::args(argc, argv);

	const uint W = 734;
	const uint H = 362;

	Flx::IResizableParent* parenth = new ResizeHandler();

	// Variables to add or replace in environ
	static const char*const envdata[] = {
		"TERM=xterm",
		"LANG=en_US.UTF-8",
		NULL
	};

	const char* shell = getenv("SHELL");
	if (shell == NULL) {
		shell = "/bin/sh";
	}
	// spawn shell in pseudo terminal
	PTY* pty = pty_spawn(shell, envdata);
	if (pty == NULL) {
		exit(EXIT_FAILURE);
	}


	// create the main window and the terminal widget
	main_win = new Fl_Double_Window(W, H);
	main_win->box(FL_NO_BOX);
	main_win->begin();

	// inner dimensions
	const int iw = W - Fl::box_dw(main_win->box());
	const int ih = H - Fl::box_dh(main_win->box());

	int x = 0 + Fl::box_dx(main_win->box());
	int y = 0 + Fl::box_dy(main_win->box());

	Flx::VT::SlaveIO* ptyio = new Flx::VT::PtyIO( pty_getfd(pty) );
	Flx::VT::ScrolledTerm* term = new Flx::VT::ScrolledTerm(parenth, ptyio, x, y, iw, ih);
	main_win->resizable(term);

	main_win->end();

	term->init();


	// show the windows
	main_win->show(argc, argv);

	int exit_res = Fl::run();

	pty_restore(pty);


	printf("Exited cleanly with status: %d\n", exit_res);
	return exit_res;
}
