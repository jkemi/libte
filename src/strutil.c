#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "strutil.h"

int str_mbr_to_cps_n(int32_t* dest, const char* locstr, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread) {
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));

	// Convert string
	size_t bytesleft = src_size;
	size_t destleft = dest_size;
	for (const char* s = locstr; destleft > 0 && bytesleft > 0; dest++) {
		wchar_t wchar;
		size_t nbytes = mbrtowc(&wchar, s, bytesleft, &ps);
		if (nbytes < 0) {
			if (nbytes == (size_t)-1) {
				// invalid sequence
				return -1;
			}
			if (nbytes == (size_t)-2) {
				// incomplete sequence
				break;
			}
		}
		if (nbytes == 0) {
			// L'\0' decoded
			return -1;
		}
		*dest = wchar;
		bytesleft -= nbytes;
		s += nbytes;
		destleft--;
	}


	*nwritten = dest_size-destleft;
	*nread = src_size-bytesleft;

	return 0;
}

