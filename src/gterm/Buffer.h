/*
 * Buffer.h
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

#include "buffersymbol.h"
#include "BufferRow.h"

typedef unsigned int uint;

class Buffer {
private:
	BufferRow**	_rows;
	uint32_t	_rowp;
	uint		_nrows;
	uint		_ncols;
private:
	void		_scrollbufStore(const BufferRow* row);
	void		_scrollbufFetch(BufferRow* row);
public:
	Buffer(uint nrows, uint ncols);
	Buffer(const Buffer&);
	virtual ~Buffer();

	void reshape(uint nrows, uint ncols);

	inline uint	getNRows() {
		return _nrows;
	}

	inline uint	getNCols() {
		return _ncols;
	}

	inline BufferRow*	getRow(int rowno) {
		return _rows[(_rowp + rowno)%_nrows];
	}

};

#endif /* BUFFER_H_ */
