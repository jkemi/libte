/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>

#include "misc.h"

#include "viewport_dirty.h"

void dirty_init(Dirty* d, uint nrows, uint ncols) {
	d->_nrows = nrows;
	d->_ncols = ncols;

	d->start = (int*)malloc(sizeof(int)*nrows);
	d->end = (int*)malloc(sizeof(int)*nrows);

	// start dirty
	for (uint i = 0; i < nrows; i++) {
		d->start[i] = 0;
		d->end[i] = ncols;
	}
}

void dirty_free(Dirty* d) {
	free(d->start);
	free(d->end);

#ifndef NDEBUG
	d->start = NULL;
	d->end = NULL;
#endif
}

void dirty_reshape(Dirty* d, uint nrows, uint ncols) {
	dirty_free(d);
	dirty_init(d, nrows, ncols);
}

void dirty_taint(Dirty* d, int row, int start, int end) {
	const int olds = d->start[row];
	const int olde = d->end[row];

	d->start[row] = int_max(0, int_min(d->start[row], start));
	d->end[row] = int_min(d->_ncols, int_max(d->end[row], end));

	if (d->start[row] >= d->end[row]) {
		d->start[row] = d->end[row] = 0;
	}

	if (olds != d->start[row] || olde != d->end[row]) {
//		printf("dirt: %d  %d,%d -> %d,%d\n", row, olds, olde, this->start[row], this->end[row]);
	}
}

void dirty_cleanse(Dirty* d, int row, int start, int end) {
	const int olds = d->start[row];
	const int olde = d->end[row];

	if (start <= d->start[row]) {
		d->start[row] = int_max(d->start[row], end);
	}

	if (end >= d->end[row]) {
		d->end[row] = int_min(d->end[row], start);
	}

	if (d->start[row] >= d->end[row]) {
		d->start[row] = d->end[row] = 0;
	}


	if (olds != d->start[row] || olde != d->end[row]) {
//		printf("dirt: %d  %d,%d -> %d,%d\n", row, olds, olde, this->start[row], this->end[row]);
	}
}
