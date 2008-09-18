/*
 * History.h
 *
 *  Created on: Aug 20, 2008
 *      Author: jakob
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "BufferRow.h"
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
void history_fetch(History* hist, BufferRow* row);

#endif /* HISTORY_H_ */
