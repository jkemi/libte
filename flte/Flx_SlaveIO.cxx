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

#include "libte/pty.h"
#include "strutil.h"

#include "Flx_SlaveIO.hpp"

//#define DEBUG_TPIPE
#undef DEBUG_TPIPE

namespace Flx {
namespace VT {

PtyIO::PtyIO(TE_Pty* pty) {
	_pty = pty;
#ifdef DEBUG_TPIPE
	_fd = open("test/tpipe", O_RDWR);
#else
	_fd = te_pty_getfd(pty);
#endif
	_fill = 0;
	Fl::add_fd(_fd, FL_READ|FL_EXCEPT, _s_fd_cb, this);
}

PtyIO::~PtyIO() {
	Fl::remove_fd(_fd);

	if (_pty != NULL) {
		te_pty_restore(_pty);
	}
}

bool PtyIO::resizeSlave(int width, int height) {
	if (_pty == NULL) {
		return false;
	}

#ifdef DEBUG_TPIPE
	return true;
#else
	return te_pty_set_window_size(_fd, width, height) == 0;
#endif
}

bool PtyIO::toSlave(const int32_t* data, int len) {
	if (_pty == NULL) {
		return false;
	}

	const size_t nbytes = MB_CUR_MAX*(len+1);
	char tmp[nbytes];

	size_t nwritten;

	int ret = str_cps_to_mbs_n(tmp, data, nbytes, len, &nwritten, NULL);
	assert (ret >= 0);

	//	str_mbs_hexdump("to pty: ", tmp, nwritten);
	size_t remaining = nwritten;
	const char* src = tmp;
	while(remaining >0) {
		ssize_t r = write(_fd, src, remaining);
		if (r < 1) {
			return false;
		}
		remaining -= r;
		src += r;
	}

	return true;
}

// Called by FLTK whenever input is received on filedes
void PtyIO::_fromSlave(int fd) {
	if (_pty == NULL) {
		SlaveIO::fromSlave(&_exit_status, 0);
		return;
	}

	ssize_t ret = read(fd, _buf+_fill, (_BUFSIZE-_fill)*sizeof(unsigned char));
	if (ret == -1 || ret == 0) {
		printf("bye from %s:%d ret: %ld\n", __FILE__, __LINE__, ret);

		Fl::remove_fd(_fd);
		_exit_status = te_pty_restore(_pty);
		_pty = NULL;

		int32_t exit_status = _exit_status;
		SlaveIO::fromSlave(&exit_status, 0);
		return;
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

		SlaveIO::fromSlave(cpbuf, cpcount);

		const size_t remaining = _fill-bytesread;
		memcpy(_buf, _buf+bytesread, remaining);
		_fill = remaining;
	}
}

}	// namespace VT
}	// namespace Flx
