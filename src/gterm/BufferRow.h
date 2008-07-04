/*
 * BufferRow.h
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#ifndef BUFFERROW_H_
#define BUFFERROW_H_

#include "buffersymbol.h"

typedef unsigned int uint;

class BufferRow {
public:
	uint		used;
	uint		capacity;
	symbol_t*	data;
private:
	void _init(uint initsize);
	void _ensureCapacity(uint capacity);
	void _pad(uint x, int len, symbol_t style);
public:
	BufferRow();
	BufferRow(uint initsize);
	BufferRow(const BufferRow&);
	virtual ~BufferRow();

	void clear(void);
	void insert(uint x, const symbol_t* symbols, uint len);
	void replace(uint x, const symbol_t* symbols, uint len);
	void fill(uint x, const symbol_t value, uint len);
	void remove(uint x, uint len);
};

#endif /* BUFFERROW_H_ */
