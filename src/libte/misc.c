/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "misc.h"


void errorf(const char* format, ...) {
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void _debugf (const char* label, const char* file, const char* func, int line, const char* format, ...)
{
	fprintf (stderr, "%s %s (%d) : %s: ", label, file, line, func);
	va_list va;
	va_start (va, format);
	vfprintf (stderr, format, va);
	va_end (va);
	fflush(stderr);
}

void* _xmalloc(const char* func, size_t size) {
	void* p = malloc(size);
	if (p == NULL) {
		errorf("ERROR: %s: xmalloc(%lu) failed with reason: %s\n", func, size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void* _xcalloc(const char* func, size_t size) {
	void* p = calloc(1,size);
	if (p == NULL) {
		errorf("ERROR: %s: xcmalloc(%lu) failed with reason: %s\n", func, size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void* _xrealloc(const char* func, void* oldptr, size_t size) {
	void* p = realloc(oldptr,size);
	if (p == NULL) {
		errorf("ERROR: %s: xrealloc(%lu) failed with reason: %s\n", func, size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void* _xmemdup(const char* func, const void* oldptr, size_t size) {
	void* p = malloc(size);
	if (p == NULL) {
		errorf("ERROR: %s: xmemdup(%lu) failed with reason: %s\n", func, size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	memcpy(p, oldptr, size);
	return p;
}

