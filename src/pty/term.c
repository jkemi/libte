/*
 * term.c
 *
 *  Created on: Aug 26, 2008
 *      Author: jakob
 */

#include <termios.h>
#include <sys/ioctl.h>

#include "term.h"

void term_set_window_size(int fd, int width, int height) {
	struct winsize ws;

	ws.ws_col = width;
	ws.ws_row = height;
	ioctl(fd, TIOCSWINSZ, &ws);
}

