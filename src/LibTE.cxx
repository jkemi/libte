/*
 * LibTE.cxx
 *
 *  Created on: Aug 25, 2008
 *      Author: jakob
 */

#include "LibTE.hpp"


void TE::teInit(int width, int height) {
	const static TE_Frontend _callbacks = {
			&_impl_draw_text,
			&_impl_draw_clear,
			&_impl_draw_cursor,
			&_impl_draw_move,

			&_impl_updated,
			&_impl_reset,
			&_impl_bell,
			&_impl_title,
			&_impl_send_back,
			&_impl_request_resize,
			&_impl_position,
	};

	_te = te_create(&_callbacks, this, width, height);
}

