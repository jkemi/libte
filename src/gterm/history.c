/*
 * History.cpp
 *
 *  Created on: Aug 20, 2008
 *      Author: jakob
 */

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "BufferRow.h"

#include "history.h"

void history_init(History* hist, uint capacity) {
	hist->capacity = capacity;
	hist->size = 0;
	hist->pos = 0;
	hist->data = (HistoryEntry*)malloc(sizeof(HistoryEntry)*capacity);
	memset(hist->data, 0, sizeof(HistoryEntry)*capacity);
}

void history_term(History* hist) {
	history_clear(hist);
	free (hist->data);
}

void history_clear(History* hist) {
	for (uint i = 0; i < hist->capacity; i++) {
		HistoryEntry* entry = hist->data+i;
		if (entry->data != NULL) {
			free (entry->data);
			entry->data = NULL;
			entry->size = 0;
		}
	}
	hist->size = 0;
	hist->pos = 0;
}

uint history_size(History* hist) {
	return hist->size;
}

uint history_peek(History* hist, uint age, symbol_t* dest, uint n) {
	if (age >= hist->capacity) {
		return 0;
	}
	const HistoryEntry* e = hist->data + (hist->pos-1-age)%hist->capacity;
	if (e->data == NULL) {
		return 0;
	}

	if (n > e->size) {
		n = e->size;
	}

	memcpy(dest, e->data, sizeof(symbol_t)*n);
	return n;
}

void history_store(History* hist, const BufferRow* row) {
	HistoryEntry* e = hist->data + hist->pos%hist->capacity;
	hist->pos++;

	if (e->data != NULL) {
		free(e->data);
	} else {
		hist->size++;
	}

	e->data = (symbol_t*)malloc(sizeof(symbol_t)*row->used);
	memcpy(e->data, row->data, sizeof(symbol_t)*row->used);
	e->size = row->used;
}

void history_fetch(History* hist, BufferRow* row) {
	hist->pos--;
	HistoryEntry* e = hist->data + hist->pos%hist->capacity;

	if (e->data != NULL) {
		hist->size--;

		bufrow_replace(row, 0, e->data, e->size);

		free(e->data);
		e->data = NULL;
		e->size = 0;
	} else {
		bufrow_clear(row);
	}
}
