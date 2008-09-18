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

class Dirty {
private:
	int		_nrows;
	int		_ncols;
public:
	int* 	start;		// first dirty column
	int*	end;		// last dirty column+1

private:
	void _init(uint nrows, uint ncols);
public:
	Dirty(uint nrows, uint ncols);
	Dirty(const Dirty& old);
	virtual ~Dirty();

	void reshape(uint nrows, uint ncols);
	void setRowDirt(int row) {start[row] = 0; end[row] = _ncols;}

	/**
	 * Set portion of row as dirty.
	 * \param start	first dirty col
	 * \param end last dirty col+1
	 */
	void setDirty(int row, int start, int end);
	void cleanseRow(int row) {start[row] = INT_MAX; end[row] = 0;}
	void cleanse(int row, int start, int end);
};

#endif /* DIRTY_H_ */
