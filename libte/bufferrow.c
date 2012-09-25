/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "misc.h"

#include "bufferrow.h"

static void _bufrow_init(BufferRow* br, uint initsize, symbol_t blank) {
	if (initsize < 32) {
		initsize = 32;
	}
	br->used = 0;
	br->capacity = initsize;
	br->data = xnew(symbol_t, initsize);
	br->erase = blank;
}

static void _bufrow_ensureCapacity(BufferRow* br, uint capacity) {
	if (capacity > br->capacity) {
		uint newcapacity = br->capacity;
		while (capacity > newcapacity) {
			newcapacity *= 2;
		}

		br->data = xreallocnew(symbol_t, br->data, newcapacity);
		br->capacity = newcapacity;
	}
}

// pad interval [x,x+len) with 'blank'
static void _bufrow_pad(BufferRow* br, uint x, int len, symbol_t blank) {
	assert (x+len <= br->capacity);

	for (uint i = x; i < x+len; i++) {
		// should fg,bg,attrs come from currently set attributes?
		br->data[i] = ' ' | blank;
	}
}

BufferRow* bufrow_new(symbol_t blank) {
	BufferRow* br = xnew(BufferRow, 1);
	_bufrow_init(br, 32, blank);
	return br;
}

void bufrow_free(BufferRow* br) {
	free(br->data);
	free(br);
}

void bufrow_clear(BufferRow* br, symbol_t blank) {
	br->used = 0;
	br->erase = blank;
}

void bufrow_trim(BufferRow* br) {
	if (br->used == 0) {
		return;
	}
	
	const symbol_t tail = br->data[br->used-1];
	if (symbol_get_codepoint(tail) != ' ') {
		return;
	}

	int trimmed=0;
	br->erase = tail;
	for (int i=br->used-1; i>=0; i--) {
		if (br->data[i] != tail) {
			break;
		}
		trimmed++;
	}
	
	br->used -= trimmed;
}

// inserts symbols at interval [x,x+len) padding with 'blank' if needed
void bufrow_insert(BufferRow* br, uint x, const symbol_t* symbols, uint len, symbol_t blank) {
	const int trail = br->used-x;
	if (trail <= 0) {
		bufrow_replace(br, x, symbols, len, blank);
	} else {
		_bufrow_ensureCapacity(br, br->used+len);
		memmove(br->data+x+len, br->data+x, sizeof(symbol_t)*trail);
		memcpy(br->data+x, symbols, sizeof(symbol_t)*len);
		br->used += len;
	}
}

// replace symbols at interval [x,x+len) with symbols
void bufrow_replace(BufferRow* br, uint x, const symbol_t* symbols, uint len, symbol_t blank) {
	if (len == 0) {
		return;
	}

	if (x+len > br->capacity) {
		_bufrow_ensureCapacity(br, x+len);
	}

	if (x > br->used) {
		_bufrow_pad(br, br->used, x-br->used, blank);
	}

	memcpy(br->data+x, symbols, sizeof(symbol_t)*len);
	br->used = uint_max(x+len, br->used);
}

void bufrow_reset(BufferRow* br, const symbol_t* symbols, uint len) {
	if (len > br->capacity) {
		_bufrow_ensureCapacity(br, len);
	}
	
	memcpy(br->data, symbols, sizeof(symbol_t)*len);
	br->used = len;
}


void bufrow_fill(BufferRow* br, uint x, const symbol_t value, uint len, symbol_t blank) {
	if (x+len > br->capacity) {
		_bufrow_ensureCapacity(br, x+len);
	}

	assert (x <= br->used);
	if (x > br->used) {
		_bufrow_pad(br, br->used, x-br->used, blank);
	}

	for (uint i = x; i < x+len; i++) {
		br->data[i] = value;
	}
	br->used = uint_max(x+len, br->used);
}

void bufrow_remove(BufferRow* br, uint x, uint len, uint width) {
	if (x >= br->used) {
		return;
	}

	const int trail = uint_min(br->used, width)-(x+len);
	if (trail > 0) {
		memmove(br->data+x, br->data+x+len, sizeof(symbol_t)*trail);
		br->used = x+trail;
	} else {
		br->used = x;
	}
}
