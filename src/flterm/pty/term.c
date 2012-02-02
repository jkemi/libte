/*
 * term.c
 *
 *  Created on: Aug 26, 2008
 *      Author: jakob
 */

#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include "term.h"

void term_set_window_size(int fd, int width, int height) {
	struct winsize ws;

	ws.ws_col = width;
	ws.ws_row = height;

	// TODO: these are unused? (seems so in Gnome's libvte and others at least...)
	ws.ws_xpixel = 0;
	ws.ws_ypixel = 0;
	printf("Setting ioctl TERM dimensions to %d, %d\n", width, height);
	ioctl(fd, TIOCSWINSZ, &ws);
}

void term_set_utf8(int fd, int utf8) {
	#if defined(IUTF8)
		struct termios tio;
		tcflag_t saved_cflag;
		if (fd != -1) {
				if (tcgetattr(fd, &tio) != -1) {
						saved_cflag = tio.c_iflag;
						tio.c_iflag &= ~IUTF8;
						if (utf8) {
								tio.c_iflag |= IUTF8;
						}
						if (saved_cflag != tio.c_iflag) {
								tcsetattr(fd, TCSANOW, &tio);
						}
				}
		}
	#endif
}
