/*
 * misc.h
 *
 *  Created on: Jul 4, 2008
 *      Author: jakob
 */

#ifndef MISC_H_
#define MISC_H_

#include <stdint.h>
#include <stddef.h>

typedef unsigned int uint;

static inline int int_min(int a, int b) {return (a < b) ? a : b;}
static inline int int_max(int a, int b) {return (a > b) ? a : b;}
static inline int uint_min(uint a, uint b) {return (a < b) ? a : b;}
static inline int uint_max(uint a, uint b) {return (a > b) ? a : b;}


static inline int int_clamp(int x, int lo, int hi) {
	if (x < lo) {
		x = lo;
	}
	if (x > hi) {
		x = hi;
	}
	return x;
}

static inline uint uint_clamp(uint x, uint lo, uint hi) {
	if (x < lo) {
		x = lo;
	}
	if (x > hi) {
		x = hi;
	}
	return x;
}

//
// Logging
//
void errorf(const char* format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));

#define DEBUGF(fmt,...) _debugf("DEBUG:   ",__FILE__,__func__,__LINE__,fmt, ##__VA_ARGS__)
#define WARNF(fmt,...)  _debugf("WARNING: ",__FILE__,__func__,__LINE__,fmt, ##__VA_ARGS__)
#define ERRORF(fmt,...) _debugf("ERROR:   ",__FILE__,__func__,__LINE__,fmt, ##__VA_ARGS__)
// error reporting as both window message and debugger string
void _debugf(const char* label, const char* file, const char* func, int line, const char* format, ...) __attribute__ ((format (printf, 5, 6)));

//
// Memory allocation/deallocation
//
void* _xmalloc(const char* func, size_t size);
#define xmalloc(s)				_xmalloc(__func__,s)
#define xnew(t,n)				(t*)xmalloc(sizeof(t)*(n))
#define xnew_aligned(t,a,n)		(t*)xmalloc_aligned(a,sizeof(t)*(n))

void* _xcalloc(const char* func, size_t size);
#define xcalloc(s)				_xcalloc(__func__,s)
#define xcnew(t,n)				(t*)xcalloc(sizeof(t)*(n))

#define allocanew(t,n)				(t*)alloca(sizeof(t)*(n))



#endif /* MISC_H_ */
