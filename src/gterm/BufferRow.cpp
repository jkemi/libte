/*
 * BufferRow.cpp
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#include <assert.h>
#include <string.h>

#include "misc.h"

#include "BufferRow.h"


void BufferRow::_init(uint initsize) {
	if (initsize < 32) {
		initsize = 32;
	}
	used = 0;
	capacity = initsize;
	data = new symbol_t[initsize];
}

void BufferRow::_ensureCapacity(uint capacity) {
	if (capacity > this->capacity) {
		uint newcapacity = this->capacity;
		while (capacity > newcapacity) {
			newcapacity *= 2;
		}

		symbol_t* newdata = new symbol_t[newcapacity];
		memcpy(newdata, data, sizeof(symbol_t)*used);

		data = newdata;
		this->capacity = newcapacity;
	}
}

void BufferRow::_pad(uint x, int len, symbol_t style) {
	assert (x+len <= capacity);

	for (uint i = x; i < x+len; i++) {
		data[i] = style;
	}
}


BufferRow::BufferRow() {
	_init(32);
}

BufferRow::BufferRow(uint initsize) {
	_init(initsize);
}

// Deliberately left out copy-constructor
//BufferRow::BufferRow(const BufferRow&) {}

BufferRow::~BufferRow() {
	delete data;
}

void BufferRow::clear() {
	used = 0;
}

void BufferRow::insert(uint x, const symbol_t* symbols, uint len) {
	const int trail = used-x;
	if (trail <= 0) {
		replace(x, symbols, len);
	} else {
		_ensureCapacity(used+len);
		memcpy(data+x+len, data+x, sizeof(symbol_t)*trail);
		memcpy(data+x, symbols, sizeof(symbol_t)*len);
		used += len;
	}
}

void BufferRow::replace(uint x, const symbol_t* symbols, uint len) {
	if (len == 0) {
		return;
	}

	if (x+len > capacity) {
		_ensureCapacity(x+len);
	}

	uint ds = x;
	if (x > used) {
		_pad(used, x-used, symbol_get_attributes(symbols[len-1]));
		ds = used;
	}

	memcpy(data+x, symbols, sizeof(symbol_t)*len);
	used = uint_max(x+len, used);
}

void BufferRow::fill(uint x, const symbol_t value, uint len) {
	if (x+len > capacity) {
		_ensureCapacity(x+len);
	}

	uint ds = x;
	if (x > used) {
		_pad(used, x-used, symbol_get_attributes(value));
		ds = used;
	}

	for (uint i = x; i < x+len; i++) {
		data[i] = value;
	}
	used = uint_max(x+len, used);
}

void BufferRow::remove(uint x, uint len) {
	if (x >= used) {
		return;
	}

	const int trail = used-(x+len);
	if (trail > 0) {
		memmove(data+x, data+x+len, sizeof(symbol_t)*trail);
		used = x+trail;
	} else {
		used = x;
	}
}
