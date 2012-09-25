/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "bufferrow.h"
#include "macros.h"

typedef struct {
	uint16_t	size;
	symbol_t*	data;
} HistoryEntry;

typedef struct  {
	HistoryEntry*	data;
	uint			size;
	uint			capacity;
	uint			pos;
} History;

void history_init(History* hist, uint capacity);
void history_term(History* hist);

void history_clear(History* hist);

uint history_size(History* hist);
uint history_peek(History* hist, uint age, symbol_t* dest, uint n);

void history_store(History* hist, const BufferRow* row);
void history_fetch(History* hist, BufferRow* row, symbol_t blank);

#endif /* HISTORY_H_ */
