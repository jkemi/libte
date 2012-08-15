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

#include "pty/pty.h"
#include "strutil.h"

#include "Flx_SlaveIO.hpp"


namespace Flx {
namespace VT {
	
PtyIO::PtyIO(int fd) : _fd(fd) {
	_fill = 0;
	Fl::add_fd(_fd, FL_READ|FL_EXCEPT, _s_fd_cb, this);
}

PtyIO::~PtyIO() {
	Fl::remove_fd(_fd);
}
	
bool PtyIO::resizeSlave(int width, int height) {
	return pty_set_window_size(_fd, width, height) == 0;
}

bool PtyIO::toSlave(const int32_t* data, int len) {
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
	ssize_t ret = read(fd, _buf+_fill, (_BUFSIZE-_fill)*sizeof(unsigned char));
	if (ret == -1 || ret == 0) {
		printf("bye from %s:%d ret: %ld\n", __FILE__, __LINE__, ret);
		// TODO: fixup exit status join() ? blah blah
		int32_t exit_status = 0;
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
