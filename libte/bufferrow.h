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
	symbol_t	erase;
} BufferRow;

BufferRow* bufrow_new(symbol_t blank);
void bufrow_free(BufferRow* br);

void bufrow_clear(BufferRow* br, symbol_t blank);
void bufrow_trim(BufferRow* br);
void bufrow_insert(BufferRow* br, uint x, const symbol_t* symbols, uint len, symbol_t blank);
void bufrow_replace(BufferRow* br, uint x, const symbol_t* symbols, uint len, symbol_t blank);
void bufrow_reset(BufferRow* br, const symbol_t* symbols, uint len);
void bufrow_fill(BufferRow* br, uint x, const symbol_t value, uint len, symbol_t blank);
void bufrow_remove(BufferRow* br, uint x, uint len, uint width);


/*
	void _init(uint initsize);
	void _ensureCapacity(uint capacity);
	void _pad(uint x, int len);
	*/

#endif /* BUFFERROW_H_ */
