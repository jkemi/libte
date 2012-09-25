/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

#include "history.h"
#include "symbol.h"
#include "bufferrow.h"
#include "macros.h"

typedef struct {
	History*	hist;
	BufferRow**	rows;
	uint		nrows;
	uint		ncols;
} Buffer;

void buffer_init(Buffer* buf, History* hist, uint nrows, uint ncols, symbol_t blank);
void buffer_term(Buffer* buf);
void buffer_clear(Buffer* buf, symbol_t blank);
void buffer_reshape(Buffer* buf, uint nrows, uint ncols, symbol_t blank);
//void buffer_scroll(Buffer* buf, uint top, uint bottom, int byoffset);
void buffer_scroll_up(Buffer* buf, uint top, uint bottom, symbol_t blank);
void buffer_scroll_down(Buffer* buf, uint top, uint bottom, symbol_t blank);

static inline BufferRow* buffer_get_row(Buffer* buf, uint rowno) {
	return buf->rows[rowno];
}

#endif /* BUFFER_H_ */
