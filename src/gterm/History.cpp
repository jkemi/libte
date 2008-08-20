/*
 * History.cpp
 *
 *  Created on: Aug 20, 2008
 *      Author: jakob
 */

#include <stddef.h>

#include "BufferRow.h"

#include "History.h"

void history_store(History* hist, const BufferRow* row) {
	if (hist != NULL) {

	}
}

void history_fetch(History* hist, BufferRow* row) {
	if (hist == NULL) {
		row->clear();
	}
}
