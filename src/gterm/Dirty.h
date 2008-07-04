/*
 * Dirty.h
 *
 *  Created on: Jul 4, 2008
 *      Author: jakob
 */

#ifndef DIRTY_H_
#define DIRTY_H_

typedef unsigned int uint;

class Dirty {
private:
	int		_nrows;
	int		_ncols;
public:
	int* 	start;
	int*	end;

private:
	void _init(uint nrows, uint ncols);
public:
	Dirty(uint nrows, uint ncols);
	Dirty(const Dirty& old);
	virtual ~Dirty();

	void reshape(uint nrows, uint ncols);
	void setRowDirt(int row) {start[row] = 0; end[row] = _ncols;}
	void setDirty(int row, int start, int end);
	void cleanse(int row) {start[row] = end[row] = 0;}
};

#endif /* DIRTY_H_ */
