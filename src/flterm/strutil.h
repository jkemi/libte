#ifndef STRUTIL_H_
#define STRUTIL_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int str_cps_to_mbs_n(char* dest, const int32_t* cps, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread);
int str_mbs_to_cps_n(int32_t* dest, const char* mbs, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread);

size_t str_cpslen(const int32_t* cps);
size_t str_mbslen(const char* mbs);

// TODO: remove this when unused...
#ifndef NDEBUG
	void str_mbs_hexdump(const char* label, const char* mbs, size_t len);
	void str_cps_hexdump(const char* label, const int32_t* cps, size_t len);
#else
	static inline void str_mbs_hexdump(const char* label, const char* mbs, size_t len) {}
	static inline void str_cps_hexdump(const char* label, const int32_t* cps, size_t len) {}
#endif

#ifdef __cplusplus
}
#endif

#endif
