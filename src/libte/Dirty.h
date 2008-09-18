/*
 * Dirty.h
 *
 *  Created on: Jul 4, 2008
 *      Author: jakob
 */

//
// Keeps track of dirty regions.
//
// Each line keeps track of dirty region by start and end.
//

#ifndef DIRTY_H_
#define DIRTY_H_

#include <limits.h>

#include "misc.h"
#include "macros.h"

typedef struct {
	int		_nrows;
	int		_ncols;
	int* 	start;		// first dirty column
	int*	end;		// last dirty column+1
} Dirty;

CDECLS_BEGIN

void dirty_init(Dirty* d, uint nrows, uint ncols);
void dirty_free(Dirty* d);

void dirty_reshape(Dirty* d, uint nrows, uint ncols);
static inline void dirty_taint_row(Dirty* d, int row) {d->start[row] = 0; d->end[row] = d->_ncols;}
static inline void dirty_cleanse_row(Dirty* d, int row) {d->start[row] = INT_MAX; d->end[row] = 0;}

/**
 * Set portion of row as dirty.
 * \param start	first dirty col
 * \param end last dirty col+1
 */
void dirty_taint(Dirty* d, int row, int start, int end);
void dirty_cleanse(Dirty* d, int row, int start, int end);

CDECLS_END

#endif /* DIRTY_H_ */