/*
 * Buffer.cpp
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#include <string.h>

#include "Buffer.h"

void buffer_init(Buffer* buf, History* hist, uint nrows, uint ncols) {
	buf->hist = hist;
	buf->rows = new BufferRow* [nrows];
	for (uint rowno = 0; rowno < nrows; rowno++) {
		buf->rows[rowno] = new BufferRow();
	}
	buf->rowp = 0;
	buf->ncols = ncols;
	buf->nrows = nrows;
}

void buffer_term(Buffer* buf) {
	for (uint rowno = 0; rowno < buf->nrows; rowno++) {
		delete buf->rows[rowno];
	}
	delete buf->rows;
}

void buffer_reshape(Buffer* buf, uint nrows, uint ncols) {
	buf->ncols = ncols;

	if (nrows == buf->nrows) {
		return;
	}

	if (nrows < buf->nrows) {
		// Buffer should shrink

		const uint shrunkby = nrows-buf->nrows;

		// Store spilled rows
		for (uint rowno = 0; rowno < shrunkby; rowno++) {
			const BufferRow* row = buffer_get_row(buf, rowno);
			history_store(buf->hist, row);
			delete row;
		}

		// resize
		BufferRow** newrows = new BufferRow*[nrows];
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno+shrunkby);
			newrows[rowno] = row;
		}

		buf->rows = newrows;
	} else {
		// Buffer should grow

		const uint grownby = buf->nrows-nrows;

		// resize
		BufferRow** newrows = new BufferRow*[nrows];
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno);
			newrows[rowno+grownby] = row;
		}

		// Fetch old rows
		for (uint rowno = grownby-1; rowno >= 0; rowno--) {
			BufferRow* row = new BufferRow();
			history_fetch(buf->hist, row);
			newrows[rowno] = row;
		}

		buf->rows = newrows;
	}

	buf->rowp = 0;
	buf->nrows = nrows;
}
