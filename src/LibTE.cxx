/*
 * LibTE.cxx
 *
 *  Created on: Aug 25, 2008
 *      Author: jakob
 */

#include "LibTE.hpp"


static void _impl_draw_text (void* priv, int x, int y, const symbol_t* symbols, int len) {
	((TE*)priv)->fe_draw_text(x, y, symbols, len);
}
static void _impl_draw_clear (void* priv, int x, int y, const symbol_color_t bg_color, int len) {
	((TE*)priv)->fe_draw_clear(bg_color, x, y, len);
}
static void _impl_draw_cursor (void* priv, symbol_color_t fg_color, symbol_color_t bg_color, symbol_attributes_t attrs, int x, int y, int32_t cp) {
	((TE*)priv)->fe_draw_cursor(fg_color, bg_color, attrs, x, y, cp);
}
static void _impl_draw_move (void* priv, int y, int height, int byoffset) {
	((TE*)priv)->fe_draw_move(y, height, byoffset);
}
static void _impl_updated (void* priv) {
	((TE*)priv)->fe_updated();
}
static void _impl_reset (void* priv) {
	((TE*)priv)->fe_reset();
}
static void _impl_bell (void* priv) {
	((TE*)priv)->fe_bell();
}
static void _impl_mouse (void* priv, int x, int y) {
	((TE*)priv)->fe_mouse(x, y);
}
static void _impl_title (void* priv, const int32_t* text) {
	((TE*)priv)->fe_title(text);
}
static void _impl_send_back (void* priv, const int32_t* data) {
	((TE*)priv)->fe_send_back(data);
}
static void _impl_request_resize (void* priv, int width, int height) {
	((TE*)priv)->fe_request_resize(width, height);
}
static void _impl_position (void* priv, int offset, int size) {
	((TE*)priv)->fe_position(offset, size);
}


const static TE_Frontend _callbacks = {
		&_impl_draw_text,
		&_impl_draw_clear,
		&_impl_draw_cursor,
		&_impl_draw_move,

		&_impl_updated,
		&_impl_reset,
		&_impl_bell,
		&_impl_mouse,
		&_impl_title,
		&_impl_send_back,
		&_impl_request_resize,
		&_impl_position,
};



void TE::teInit(int width, int height) {
	_te = te_create(&_callbacks, this, width, height);
}


TE::~TE() {
	if (_te != NULL) {
		te_destroy(_te);
	}
}
