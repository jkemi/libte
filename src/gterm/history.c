/*
 * History.cpp
 *
 *  Created on: Aug 20, 2008
 *      Author: jakob
 */

#include <stddef.h>
#include <assert.h>

#include "BufferRow.h"

#include "history.h"

uint history_size(History* hist) {
	return 0;
}

uint history_peek(History* hist, uint age, symbol_t* dest, uint n) {
	assert (age == 0);
	return 0;
}

void history_store(History* hist, const BufferRow* row) {
	if (hist != NULL) {

	}
}

void history_fetch(History* hist, BufferRow* row) {
	if (hist == NULL) {
		bufrow_clear(row);
	}
}
