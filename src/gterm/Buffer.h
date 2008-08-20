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

class Buffer {
private:
	History*	_hist;

	BufferRow**	_rows;
	uint32_t	_rowp;
	uint		_nrows;
	uint		_ncols;
private:

public:
	Buffer(History* hist, uint nrows, uint ncols);
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
