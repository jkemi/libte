/*
 * Buffer.h
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

#include "History.h"
#include "buffersymbol.h"
#include "BufferRow.h"

typedef struct {
	History*	hist;
	BufferRow**	rows;
	uint32_t	rowp;
	uint		nrows;
	uint		ncols;
} Buffer;

void buffer_init(Buffer* buf, History* hist, uint nrows, uint ncols);
void buffer_term(Buffer* buf);
void buffer_reshape(Buffer* buf, uint nrows, uint ncols);

static inline BufferRow* buffer_get_row(Buffer* buf, uint rowno) {
	return buf->rows[(buf->rowp + rowno)%buf->nrows];
}


#endif /* BUFFER_H_ */
