#ifndef STRUTIL_H_
#define STRUTIL_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int str_mbr_to_cps_n(int32_t* dest, const char* locstr, size_t dest_size, size_t src_size, size_t* nwritten, size_t* nread);

#ifdef __cplusplus
}
#endif

#endif
