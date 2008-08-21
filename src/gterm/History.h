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

typedef struct _History {
} History;

CDECLS_BEGIN

uint history_size(History* hist);
uint history_peek(History* hist, uint age, symbol_t* dest, uint n);

void history_store(History* hist, const BufferRow* row);
void history_fetch(History* hist, BufferRow* row);

CDECLS_END

#endif /* HISTORY_H_ */
