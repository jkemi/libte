/*
 * viewport.h
 *
 *  Created on: Aug 21, 2008
 *      Author: jakob
 */

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include "misc.h"
#include "macros.h"

CDECLS_BEGIN

void viewport_init	(GTerm* gt, uint w, uint h);
void viewport_term	(GTerm* gt);

void viewport_reshape	(GTerm* gt, uint w, uint h);

void viewport_taint		(GTerm* gt, uint y, uint start_x, uint end_x);
void viewport_taint_all	(GTerm* gt);
void viewport_move		(GTerm* gt, uint y, uint n, int offset);

void viewport_history_inc(GTerm* gt);
void viewport_history_dec(GTerm* gt);

void viewport_set		(GTerm* gt, uint offset);


void viewport_request_redraw(GTerm* gt, int x, int y, int w, int h, bool force);

CDECLS_END

#endif /* VIEWPORT_H_ */
