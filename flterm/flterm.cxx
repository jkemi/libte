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
#include <assert.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>

#undef SYSMENU

#include "flte/Flx_ScrolledTerm.hpp"

#include "libte/LibTE.hpp"

#ifndef NDEBUG
static void _debugf (const char* label, const char* file, const char* func, int line, const char* format, ...)
{
	fprintf (stderr, "%s %s (%d) : %s: ", label, file, line, func);
	va_list va;
	va_start (va, format);
	vfprintf (stderr, format, va);
	va_end (va);
	fflush(stderr);
}

#	define DEBUGF(fmt,...) _debugf("DEBUG:   ",__FILE__,__func__,__LINE__,fmt, ##__VA_ARGS__)
#else
#	define DEBUGF(fmt,...)
#endif

static void _spawnTerm_cb(Fl_Widget* w, void* data);
static void _closeTerm_cb(Fl_Widget* w, void* data);
static void _closeApp_cb(Fl_Widget* w, void* data);

#ifdef SYSMENU

#include <FL/Fl_Sys_Menu_Bar.H>

static const Fl_Menu_Item	_menuitems[] = {
	{"&File", 0, 0, 0, FL_SUBMENU},
		{"New", FL_COMMAND+'t', &_spawnTerm_cb, NULL},
		{"Close", FL_COMMAND+'w', &_closeTerm_cb, NULL},
//		{"Quit", FL_COMMAND+'q', &_closeApp_cb, NULL},
		{0},
	{0}
};

Fl_Sys_Menu_Bar*	_sysmenu;

#endif


class TermWindow : public Fl_Double_Window, public Flx::IResizableParent {

private:
	Flx::VT::ScrolledTerm*	_term;
	Flx::VT::SlaveIO*		_ptyio;
	
public:
	
	/**
	 * claims ownership of pty
	 */
	TermWindow(TE_Pty* pty, int W, int H, const int argc, const char* const* argv) : Fl_Double_Window(W, H){
		this->box(FL_NO_BOX);
		this->begin();
				
		// inner dimensions
		const int iw = W - Fl::box_dw(this->box());
		const int ih = H - Fl::box_dh(this->box());
		
		int x = 0 + Fl::box_dx(this->box());
		int y = 0 + Fl::box_dy(this->box());
		
		_ptyio = new Flx::VT::PtyIO(pty);
		_term = new Flx::VT::ScrolledTerm(this, _ptyio, x, y, iw, ih);
		this->resizable(_term);
		
		this->end();
		
		_term->init();
	}
	
	virtual ~TermWindow() {
		delete(_term);
		delete(_ptyio);
	}
	
	int handle(int event) {
		switch(event) {
			case FL_KEYDOWN:
				// does currently only work with shortcuts containing "command"
#ifndef SYSMENU
#ifdef __APPLE__
				if (Fl::event_command()) {
#else
				if (Fl::event_ctrl() && Fl::event_shift()) {
#endif	// __APPLE__
					switch(Fl::event_key()) {
					case 't':	_spawnTerm_cb(this, NULL);	return 1;
					case 'w':	_close();					return 1;
					case 'q':	_closeApp_cb(this, NULL);	return 1;
					}
				}
#endif	// SYSMENU
				if (_term->handle(event)) {
					return 1;
				}
				break;
			default:
				break;
		}
		
		return Fl_Double_Window::handle(event);
	}
	
public:
		
	void _close() {
		delete(this);
	}
	
	void event_size_range(int minw, int minh, int maxw, int maxh, int stepw, int steph) {
		const int dw = Fl::box_dw(this->box());
		const int dh = Fl::box_dh(this->box());

		minw += dw;
		maxw += dw;
		minh += dh;
		maxh += dh;

		this->size_range(minw, minh, maxw, maxh, stepw, steph);
	}

	void event_want_size(int width, int height) {
		const int dw = Fl::box_dw(this->box());
		const int dh = Fl::box_dh(this->box());

		width += dw;
		height += dh;

		DEBUGF("SH:My size is %d,%d, wanted %d,%d\n", this->w(), this->h(), width, height);

		this->size(width, height);
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
		this->copy_label(tmp);
	}

	void event_childexit(int exit_status) {
		DEBUGF("child exited with status: %d\n", exit_status);
		_close();
	};
};
	
static void _spawnTerm_cb(Fl_Widget* w, void* data) {
	DEBUGF("spawn term\n");

	
	// Variables to add or replace in environ
	static const char*const envextra[] = {
		"TERM=xterm-color",
		"LANG=en_US.UTF-8",
		NULL
	};
	
	const char* shell = getenv("SHELL");
	if (shell == NULL) {
		shell = "/bin/sh";
	}
	
	const char* args[] = {
		shell,
		"-l",
		NULL
	};
	char** env = te_pty_env_augment(envextra);
	if (env == NULL) {
		fl_alert("unable to augment environment");
		return;
	}
	// spawn shell in pseudo terminal
	char* err = NULL;
	TE_Pty* pty = te_pty_spawn(shell, args, env, &err);
	te_pty_env_free(env);
	if (pty == NULL) {
		if (err) {
			fl_alert("unable to open slave: %s", err);
			free(err);
		} else {
			fl_alert("unable to open slave");
		}
		return;
	}

	
	const uint W = 734;
	const uint H = 362;
	TermWindow* wnd = new TermWindow(pty, W, H, 0, NULL);
	wnd->show();
}

static void _closeTerm_cb(Fl_Widget* w, void* data) {
	DEBUGF("close term\n");
	Fl_Window* window = Fl::first_window();
	if (window) {
		delete(window);
	}
}

	
static void _closeApp_cb(Fl_Widget* w, void* data) {
	DEBUGF("close app\n");
	Fl_Window* window;
	while ( (window=Fl::first_window()) ) {
		delete(window);
	}
}
	
static int _globalEventHandler(int event) {
	switch(event) {
	case FL_SHORTCUT:
#ifdef __APPLE__
			if (Fl::event_command()) {
#else
			if (Fl::event_ctrl() && Fl::event_shift()) {
#endif
				switch(Fl::event_key()) {
					case 't':	_spawnTerm_cb(NULL, NULL);	return 1;
					case 'q':	_closeApp_cb(NULL,NULL);	return 1;
					case 'w':	_closeTerm_cb(NULL,NULL);	return 1;
				}
			}
			DEBUGF("global shortcut skipped: '%c'\n", Fl::event_key());
	}
	return 0;
}

int main(int argc, char** argv)
{
	DEBUGF("libte compile version: %s, linked version: %s\n", TE_HEADER_VERSION, te_binary_version_string);

	// TODO: make configurable? or use "" for default?
	if (setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
		exit(EXIT_FAILURE);
	}
	Fl::args(argc, argv);

#ifdef SYSMENU
	_sysmenu = new Fl_Sys_Menu_Bar (0, 0, 0, 0, NULL);
	_sysmenu->menu(_menuitems);
#endif
	
	Fl::add_handler(&_globalEventHandler);

	_spawnTerm_cb(NULL, NULL);
	
	int exit_res = Fl::run();
	DEBUGF("Exited cleanly with status: %d\n", exit_res);
	return exit_res;
}
