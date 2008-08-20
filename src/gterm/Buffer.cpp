/*
 * Buffer.cpp
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#include <string.h>

#include "Buffer.h"


Buffer::Buffer(History* hist, uint nrows, uint ncols) {
	_hist = hist;
	_rows = new BufferRow* [nrows];
	for (uint rowno = 0; rowno < nrows; rowno++) {
		_rows[rowno] = new BufferRow();
	}
	_rowp = 0;
	_ncols = ncols;
	_nrows = nrows;
}

/* Deliberately left out copy-constructor
Buffer::Buffer(const Buffer& old) {
}
*/

Buffer::~Buffer() {
	for (uint rowno = 0; rowno < _nrows; rowno++) {
		delete _rows[rowno];
	}
	delete _rows;
}

void Buffer::reshape(uint nrows, uint ncols) {
	_ncols = ncols;

	if (nrows == _nrows) {
		return;
	}

	if (nrows < _nrows) {
		uint shrunkby = nrows-_nrows;

		// Store spilled rows
		for (uint rowno = 0; rowno < shrunkby; rowno++) {
			const BufferRow* row = getRow(rowno);
			history_store(_hist, row);
			delete row;
		}

		// resize
		BufferRow** newrows = new BufferRow*[nrows];
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = getRow(rowno+shrunkby);
			newrows[rowno] = row;
		}

		_rows = newrows;
	} else {
		uint grownby = _nrows-nrows;

		// resize
		BufferRow** newrows = new BufferRow*[nrows];
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = getRow(rowno);
			newrows[rowno+grownby] = row;
		}

		// Fetch old rows
		for (uint rowno = grownby-1; rowno >= 0; rowno--) {
			BufferRow* row = new BufferRow();
			history_fetch(_hist, row);
			newrows[rowno] = row;
		}

		_rows = newrows;
	}

	_rowp = 0;
	_nrows = nrows;
}
