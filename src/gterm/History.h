/*
 * History.h
 *
 *  Created on: Aug 20, 2008
 *      Author: jakob
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "macros.h"

class BufferRow;

CDECLS_BEGIN

typedef struct {

} History;

void history_store(History* hist, const BufferRow* row);
void history_fetch(History* hist, BufferRow* row);

CDECLS_END

#endif /* HISTORY_H_ */
