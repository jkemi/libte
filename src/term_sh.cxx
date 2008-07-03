#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>

#include "Fl_Term.h"

#include "pseudo/pseudo.hpp"

#ifdef __APPLE__
	static int def_fnt_size = 16;
#else
	static int def_fnt_size = 15;
#endif

// set this to show the diagnostic raw text output view
//#define SHOW_DIAG

// Set this to use deffered rather than direct terminal updates
//#define USE_DEFERUPDATE

/************************************************************************/
Fl_Double_Window *main_win = NULL;
Fl_Double_Window *diag_win = NULL;
Fl_Term *termBox;
text_box *listall;
int pty_fd = -1;

/************************************************************************/

/**
 * Called whenever input is received from pty process.
 */
void mfd_cb(int mfd, void *v)
{
	gterm_if *termIO = (gterm_if *)v;
	unsigned char buf[1000];
	int i = read(mfd, buf, 1000);
	if (i > 0)
		termIO->ProcessInput(i, buf);

	if (i <= 0) // Possible exit...
	{
		main_win->hide();
		if (diag_win) diag_win->hide();
	}
}

/************************************************************************/
void upd_term_cb(void *v)
{
	gterm_if *termIO = (gterm_if *)v;
	termIO->Update();
	Fl::repeat_timeout(0.1, upd_term_cb, v);
}
/************************************************************************/
void quit_cb(Fl_Button *, void *)
{
	char str[] = "exit\n";
	int count = strlen(str);
	write(pty_fd, str, count);

	usleep(100000); // 100ms
	main_win->hide();

	if (diag_win) diag_win->hide();
}

/************************************************************************/
int main(int argc, char **argv)
{
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
	main_win = new Fl_Double_Window(tw+10, th+60);
	main_win->begin();

	termBox = new Fl_Term(def_fnt_size, 5, 5, tw, th);
	termBox->box(FL_DOWN_BOX);

	// create some buttons for controlling the widget
	Fl_Group * but_grp = new Fl_Group(5, th+8, tw, 40);
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
	Fl::visible_focus(0);

// DO NOT set a resizable on the terminal window - it does not work right yet!
//	main_win->resizable(termBox);
//	main_win->size_range(500, 200, 0, 0, 0, 0, 0);

	// obtain a reference to the underlying GTerm object - should not need to do this, the Fl_Term
	// ought to be the only object the user has to see... Multiple inheritance maybe?
	gterm_if *termIO = termBox->get_gterm();

#ifdef SHOW_DIAG
	// create the diagnostic window and list text widget
	diag_win = new Fl_Double_Window(600, 600);
	diag_win->begin();

	listall = new text_box(5, 5, 590, 590);
	listall->box(FL_DOWN_BOX);

	diag_win->end();
	diag_win->resizable(listall);

	// Add the diagnostic window to the Fl_Term
	termBox->set_listview(listall);
#endif

	// show the windows
	main_win->show(argc, argv);
#ifdef SHOW_DIAG
	diag_win->show();
#endif

	// Give the terminal the focus by default
	Fl::focus(termBox);

	/* spawn shell in pseudo terminal */
	mfd = spawn("/bin/sh");
	if (mfd < 0) {
		exit(-1);
	}

	pty_fd = mfd;

	/* we want non-blocking reads from pty output */
	fcntl(mfd, F_SETFL, O_NONBLOCK);

	// Attach the GTerm terminal i/o to the fd...
	termIO->set_write_fd(mfd);

	/* configure terminal for deferred display updates */
#ifdef USE_DEFERUPDATE
	termIO->set_mode_flag(GTerm::DEFERUPDATE);   // enable deffered update
	// basic timeout to poll the terminal for refresh - do this if we select DEFERUPDATE
	Fl::add_timeout(0.3, upd_term_cb, (void *)termIO);
#else
	termIO->clear_mode_flag(GTerm::DEFERUPDATE); // disable deffered - direct updates are used
#endif

//	termIO->set_mode_flag(GTerm::TEXTONLY);    // enable "Text Only" mode
	termIO->clear_mode_flag(GTerm::TEXTONLY);  // disable "Text Only" mode

	termIO->set_mode_flag(GTerm::NOEOLWRAP);   // disable line wrapping
	termIO->clear_mode_flag(GTerm::LOCALECHO); // disable local echo

	// add the pty to the fltk fd list, so we can catch any output
	Fl::add_fd(mfd, mfd_cb, (void *)termIO);

	// draw the first screen
	termIO->Update();

	int exit_res = Fl::run();
	restore_ttyp();
	return exit_res;
}

/* End of File */
