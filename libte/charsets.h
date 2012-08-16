/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef CHARSETS_H_
#define CHARSETS_H_


#include <stdint.h>

typedef struct  {
	uint8_t		from;
	uint16_t	to;
} te_chartable_entry_t;

extern const te_chartable_entry_t chartable_us[];		// 0 - US ASCII
extern const te_chartable_entry_t chartable_uk[];		// A - UK ('$' -> '£')
extern const te_chartable_entry_t chartable_special[];	// 0 - DEC Special Character and Line Drawing Set

#endif // CHARSETS_H_
