/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef BUFFERROW_H_
#define BUFFERROW_H_

#include "misc.h"
#include "symbol.h"
#include "macros.h"

typedef struct {
	uint		used;
	uint		capacity;
	symbol_t*	data;
} BufferRow;

BufferRow* bufrow_new(void);
void bufrow_free(BufferRow* br);

void bufrow_clear(BufferRow* br);
void bufrow_insert(BufferRow* br, uint x, const symbol_t* symbols, uint len);
void bufrow_replace(BufferRow* br, uint x, const symbol_t* symbols, uint len);
void bufrow_fill(BufferRow* br, uint x, const symbol_t value, uint len);
void bufrow_remove(BufferRow* br, uint x, uint len, uint width);


/*
	void _init(uint initsize);
	void _ensureCapacity(uint capacity);
	void _pad(uint x, int len);
	*/

#endif /* BUFFERROW_H_ */
