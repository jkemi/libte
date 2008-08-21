/*
 * Buffer.h
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

#include "history.h"
#include "symbol.h"
#include "BufferRow.h"
#include "macros.h"

typedef struct {
	History*	hist;
	BufferRow**	rows;
	uint		nrows;
	uint		ncols;
} Buffer;

CDECLS_BEGIN

void buffer_init(Buffer* buf, History* hist, uint nrows, uint ncols);
void buffer_term(Buffer* buf);
void buffer_reshape(Buffer* buf, uint nrows, uint ncols);
//void buffer_scroll(Buffer* buf, uint top, uint bottom, int byoffset);
void buffer_scroll_up(Buffer* buf, uint top, uint bottom);
void buffer_scroll_down(Buffer* buf, uint top, uint bottom);

static inline BufferRow* buffer_get_row(Buffer* buf, uint rowno) {
	return buf->rows[rowno];
}

CDECLS_END

#endif /* BUFFER_H_ */
