/*
 * This file is part of libTE, please consult the files README and
 * COPYING for further information.
 *
 * libTE is copyright (c) 2008 by Jakob Kemi.
 */

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include "misc.h"

typedef struct Viewport_ Viewport;

void viewport_init	(TE* gt, uint w, uint h);
void viewport_term	(TE* gt);

void viewport_reshape	(TE* gt, uint w, uint h);

/**
 * Mark portions of line y dirty.
 * \param y	line to taint
 * \param start_x	first dirty col
 * \param len		number to taint
 */
void viewport_taint		(TE* gt, uint y, uint start_x, uint end_x);
void viewport_taint_all	(TE* gt);
void viewport_move		(TE* gt, uint y, uint n, int offset);

void viewport_history_inc(TE* gt);
void viewport_history_dec(TE* gt);

void viewport_set		(TE* gt, int offset);
void viewport_lock_scroll (TE* gt, bool lock);

void viewport_request_redraw(TE* gt, int x, int y, int w, int h, bool force);

#endif /* VIEWPORT_H_ */
