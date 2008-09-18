/*
 * Buffer.cpp
 *
 *  Created on: Jul 3, 2008
 *      Author: jakob
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "Buffer.h"

void buffer_init(Buffer* buf, History* hist, uint nrows, uint ncols) {
	buf->hist = hist;
	buf->rows = (BufferRow**)malloc(sizeof(BufferRow*)*nrows);
	for (uint rowno = 0; rowno < nrows; rowno++) {
		buf->rows[rowno] = bufrow_new();
	}
	buf->ncols = ncols;
	buf->nrows = nrows;
}

void buffer_term(Buffer* buf) {
	for (uint rowno = 0; rowno < buf->nrows; rowno++) {
		bufrow_free(buf->rows[rowno]);
	}
	free (buf->rows);
}

void buffer_reshape(Buffer* buf, uint nrows, uint ncols) {
	buf->ncols = ncols;

	if (nrows == buf->nrows) {
		return;
	}

	BufferRow** newrows = (BufferRow**)malloc(sizeof(BufferRow*)*nrows);
	if (nrows < buf->nrows) {
		// Buffer should shrink

		const int shrunkby = buf->nrows - nrows;

		// Store spilled rows
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno);
			newrows[rowno] = row;
		}


		for (uint rowno = nrows; rowno < buf->nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno);
			bufrow_free(row);
		}

/*
		// Store spilled rows
		for (uint rowno = 0; rowno < shrunkby; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno);
			history_store(buf->hist, row);
			bufrow_free(row);
		}

		// resize
		for (uint rowno = 0; rowno < nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno+shrunkby);
			newrows[rowno] = row;
		}
*/
	} else {
		// Buffer should grow

		// resize
		for (uint rowno = 0; rowno < buf->nrows; rowno++) {
			BufferRow* row = buffer_get_row(buf, rowno);
			newrows[rowno] = row;
		}

		// Create new rows
		for (int rowno = buf->nrows; rowno < nrows; rowno++) {
			BufferRow* row = bufrow_new();
			newrows[rowno] = row;
		}
	}

	free(buf->rows);
	buf->rows = newrows;
	buf->nrows = nrows;
}

void buffer_scroll_up(Buffer* buf, uint top, uint bottom) {
	assert (bottom > top);
	assert (bottom < buf->nrows);

	BufferRow* tmp = buf->rows[top];
	history_store(buf->hist, tmp);
	for (uint y = top; y < bottom; y++) {
		buf->rows[y] = buf->rows[y+1];
	}

	bufrow_clear(tmp);
	buf->rows[bottom] = tmp;

/*
	if (byoffset == 0) {
		return;
	}

	if (byoffset > 0) {
		// scroll up by offset off
		const uint off = byoffset;

		for (uint y = top; y < top+off; y++) {
			history_store(buf->hist, buf->rows[y]);
			buf->rows[y+off]
			buf->rows[y] = buf->rows[y+off];
		}
		for (uint y = top+off; y <= bottom-off; y++) {
			buf->rows[y] = buf->rows[y+off];
		}
		for (uint y = bottom-off+1; y <= bottom; y++) {
			buf->rows[y]->clear();
		}
	} else {
		// scroll down by offset off
		const uint off = -byoffset;

		for (uint y = bottom; y >= bottom-off; y--) {
			buf->rows[y]->clear();
		}
		for (uint y = bottom-off-1; y >= top+off; y--) {
			buf->rows[y] = buf->rows[y-off];
		}
		for (uint y = top+off-1; y >= top; y--) {
			history_fetch(buf->hist, buf->rows[y]);
		}
	}
*/
}

void buffer_scroll_down(Buffer* buf, uint top, uint bottom) {
	assert (bottom > top);
	assert (bottom < buf->nrows);

	BufferRow* tmp = buf->rows[bottom];
	for (uint y = bottom; y > top; y--) {
		buf->rows[y] = buf->rows[y-1];
	}
	history_fetch(buf->hist, tmp);
	buf->rows[top] = tmp;
}