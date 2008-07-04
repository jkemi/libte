/*
 * Dirty.cpp
 *
 *  Created on: Jul 4, 2008
 *      Author: jakob
 */

#include <string.h>

#include "misc.h"

#include "Dirty.h"



void Dirty::_init(uint nrows, uint ncols) {
	_nrows = nrows;
	_ncols = ncols;

	start = new int[nrows];
	end = new int[nrows];

	memset(start, 0, sizeof(int)*nrows);
	memset(end, 0, sizeof(int)*nrows);
}

Dirty::Dirty(uint nrows, uint ncols) {
	_init(nrows, ncols);
}

// Delibirately left out copy constructor
//Dirty::Dirty(const Dirty& old) {}

Dirty::~Dirty() {
	delete start;
	delete end;
}


void Dirty::reshape(uint nrows, uint ncols) {
	delete start;
	delete end;

	_init(nrows, ncols);
}

void Dirty::setDirty(int row, int start, int end) {
	this->start[row] = int_max(0, int_min(this->start[row], start));
	this->end[row] = int_min(_ncols, int_max(this->end[row], end));
}

