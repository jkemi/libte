#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "strutil.h"

// TODO: this won't work if sizeof(wchar_t) != 4 (i.e. Windows)
int str_cps_to_mbs_n(char* dest, const int32_t* cps, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread) {
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));

	// Convert string
	size_t bytesleft = dest_size;
	size_t srcleft = src_size;
	char buf[MB_CUR_MAX];
	for (const int32_t* s = cps; srcleft > 0 && bytesleft > 0; s++) {
		size_t nbytes = wcrtomb(buf, *s, &ps);
		if (nbytes == (size_t)-1) {
			// invalid sequence
			return -1;
		}
		if (nbytes > bytesleft) {
			break;
		}
		memcpy(dest, buf, nbytes);
		dest += nbytes;
		bytesleft -= nbytes;
		srcleft--;
	}

	if (nwritten != NULL) {
		*nwritten = dest_size-bytesleft;
	}
	if (nread != NULL) {
		*nread = src_size-srcleft;
	}

	return 0;
}


// TODO: this won't work if sizeof(wchar_t) != 4 (i.e. Windows)
int str_mbs_to_cps_n(int32_t* dest, const char* mbs, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread) {
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));

	// Convert string
	size_t bytesleft = src_size;
	size_t destleft = dest_size;
	for (const char* s = mbs; destleft > 0 && bytesleft > 0; dest++) {
		wchar_t wchar;
		size_t nbytes = mbrtowc(&wchar, s, bytesleft, &ps);
		if (nbytes == (size_t)-1) {
			// invalid sequence
			return -1;
		}
		if (nbytes == (size_t)-2) {
			// incomplete sequence
			break;
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


	if (nwritten != NULL) {
		*nwritten = dest_size-destleft;
	}
	if (nread != NULL) {
		*nread = src_size-bytesleft;
	}

	return 0;
}

size_t str_cpslen(const int32_t* cps) {
	size_t len = 0;
	for (; *cps != L'\0'; len++, cps++) {
	}
	return len;
}

// TODO: this won't work if sizeof(wchar_t) != 4 (i.e. Windows)
size_t str_mbslen(const char* mbs) {
	mbstate_t ps;
	memset(&ps, 0, sizeof(mbstate_t));

	const size_t ret = mbsrtowcs(NULL, &mbs, 0, &ps);
	assert (ret != (size_t)-1);

	return ret;
}

// TODO: remove this when unused...
#include <stdio.h>

// TODO: remove this when unused...
void str_mbs_hexdump(const char* label, const char* mbs, size_t len) {
	printf("%s", label);
	for (unsigned int i = 0; i < len; i++) {
		printf("0x%02x ", (unsigned char)mbs[i]);
	}
	printf("\n");
}

// TODO: remove this when unused...
void str_cps_hexdump(const char* label, const int32_t* cps, size_t len) {
	printf("%s", label);
	for (unsigned int i = 0; i < len; i++) {
		printf("0x%04x ", cps[i]);
	}
	printf("\n");
}
