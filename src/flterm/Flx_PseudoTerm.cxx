#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include <locale.h>
#include <string.h>

#include <stdint.h>

#include <FL/Fl.H>

#include "Flx_ScrolledTerm.hpp"

#include "pty/pty.h"
#include "pty/term.h"

#include "strutil.h"

#include "Flx_PseudoTerm.hpp"

#define BUFSIZE		1024

namespace Flx {
namespace VT {

namespace impl {

class PseudoTermPriv {
public:
	friend class Flx::VT::PseudoTerm;


protected:
	PseudoTerm*			_term;
	int 	_fd;
	char	_buf[BUFSIZE];
	size_t	_fill;

public:

	PseudoTermPriv(PseudoTerm* term, int fd) { // , IResizableParent* parenth) {
		_fd = fd;
		_fill = 0;
		_term = term;

		// add the pty to the fltk fd list, so we can catch any output
		Fl::add_fd(_fd, FL_READ|FL_EXCEPT, mfd_cb, this);
		// we want non-blocking reads from pty output
//		fcntl(_fd, F_SETFL, O_NONBLOCK);
	}

	~PseudoTermPriv() {
		Fl::remove_fd(_fd);
	}



	// Called whenever input is received from pty process.
	static void mfd_cb(int mfd, void* priv) {
		((PseudoTermPriv*)priv)->got_child_input_cb(mfd);
	}

	// Called whenever input is received from pty process.
	void got_child_input_cb(int mfd) {
		ssize_t ret = read(mfd, _buf+_fill, (BUFSIZE-_fill)*sizeof(unsigned char));
		if (ret == -1 || ret == 0) {
			printf("bye from %s:%d ret: %ld\n", __FILE__, __LINE__, ret);
			// TODO: hello
			exit(0);
			//return;
		}

		size_t bytesread = ret;
	//	str_mbs_hexdump("from pty(mbs): ", buf+buffill, bytesread);

		_fill += bytesread;

		int32_t	cpbuf[1024];
		size_t cpcount;


		if (str_mbs_to_cps_n(cpbuf, _buf, 1024, _fill, &cpcount, &bytesread) != 0) {
			//TODO: this happens.. try pilned sedan "å" så skiter det sig nog..
			_fill = 0;
			return;
	//		main_win->hide();
	//		abort();
		} else {
	//		str_cps_hexdump("from pty: ", cpbuf, cpcount);

			_term->ScrolledTerm::fromChild(cpbuf, cpcount);

			const size_t remaining = _fill-bytesread;
			memcpy(_buf, _buf+bytesread, remaining);
			_fill = remaining;
		}
	}

};


}	// namespace impl



PseudoTerm::PseudoTerm(	IResizableParent* resizable, PTY* pty, int X, int Y, int W, int H
)
	// Parent constructors
	: ScrolledTerm(resizable, this, X, Y, W, H)
	, _impl(new impl::PseudoTermPriv(this, pty_getfd(pty)))

{
}

PseudoTerm::~PseudoTerm() {
	delete _impl;
}

// IChildHandler interface

void PseudoTerm::child_sendto(const int32_t* data, int len) {
	const size_t nbytes = MB_CUR_MAX*(len+1);
	char tmp[nbytes];

	size_t nwritten;

	int ret = str_cps_to_mbs_n(tmp, data, nbytes, len, &nwritten, NULL);
	assert (ret >= 0);

//	str_mbs_hexdump("to pty: ", tmp, nwritten);
	write(_impl->_fd, tmp, nwritten);
}

void PseudoTerm::child_resize(int width, int height) {
	term_set_window_size(_impl->_fd, width, height);
}

}	// namespace VT
}	// namespace Flx
