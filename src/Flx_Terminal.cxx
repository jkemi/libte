/*
 * Flx_Terminal.cxx
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#include <Fl/Fl.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Scrollbar.H>

#include <stdio.h>

#include "Fl_Term.h"

#include "Flx_Terminal.hpp"

typedef unsigned int uint;

#define SCROLLBAR_WIDTH		12


class Flx_Terminal_Impl {
public:
	void (*_to_child_cb)(const int32_t* data, size_t size, void* priv);
	void* _to_child_cb_data;

	Fl_Term*		term;
	Fl_Scrollbar*	scrollbar;

public:
	// Called by FLTK widget actions
	void _widget_cb(Fl_Widget* widget) {
		if (widget == term) {
			printf( "term clicked!\n" );
		} else if (widget == scrollbar) {
			printf( "scrollbar: %d\n", scrollbar->value());
			scrollbar->redraw();
			te_position(term->_te, scrollbar->value());
		}
	}

	// Called by libte on scroll positioning
	void _scroll_cb(int offset, int size) {
		if (size == 0) {
			scrollbar->minimum(100);
			scrollbar->maximum(0);
			scrollbar->slider_size(1.0);
			scrollbar->deactivate();
		} else {
			scrollbar->minimum(size);
			scrollbar->maximum(0);
			scrollbar->slider_size(1.0/(size+1));
			scrollbar->Fl_Slider::value((double)offset);

			// TODO: this isn't honored!?
			scrollbar->step(1.0);

			scrollbar->activate();

			scrollbar->redraw();
		}
	}

	// Called by Fl_Term on sendback
	void _send_back_cb(const int32_t* data) {
		size_t n;
		for (n = 0; data[n] != 0; n++) {

		}
		toChild(data, n);
	}

private:
	void toChild(const int32_t* data, size_t size) {
		if (_to_child_cb != 0) {
			_to_child_cb(data, size, _to_child_cb_data);
		}
	}
};

// Called by FLTK widget actions
static void _widget_cb(Fl_Widget* widget, void* priv) {
	((Flx_Terminal_Impl*)priv)->_widget_cb(widget);
}

// Called by libte on scroll positioning
static void _scroll_cb(void* priv, int offset, int size) {
	((Flx_Terminal_Impl*)priv)->_scroll_cb(offset, size);
}

// Called by Fl_Term on sendback
static void _send_back_cb(void* priv, const int32_t* data) {
	((Flx_Terminal_Impl*)priv)->_send_back_cb(data);
}

Flx_Terminal::Flx_Terminal(int X, int Y, int W, int H, const char* label) : Fl_Group(X,Y,W,H) {
	_impl = new Flx_Terminal_Impl();

	// inner dimensions
	const uint iw = W - Fl::box_dw(box());
	const uint ih = H - Fl::box_dh(box());

	uint x = X + Fl::box_dx(box());
	uint y = Y + Fl::box_dy(box());


	_impl->term = new Fl_Term(15, x, y, iw-SCROLLBAR_WIDTH, ih);
	x += iw-SCROLLBAR_WIDTH;

	_impl->scrollbar = new Fl_Scrollbar(x, y, SCROLLBAR_WIDTH, ih);
	_impl->scrollbar->step(1);
	_impl->scrollbar->callback(&_widget_cb, _impl);

	end();

	_impl->term->set_scroll_func(&_scroll_cb, _impl);
	_impl->term->set_send_back_func(&_send_back_cb, _impl);

	_impl->_scroll_cb(0, 0);
}

Flx_Terminal::~Flx_Terminal() {
	delete _impl->scrollbar;
	delete _impl->term;
	delete _impl;
}

int Flx_Terminal::handle(int event) {
	return Fl_Group::handle(event);
/*	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	case FL_KEYBOARD: {
		if (_handle_keyevent()) {
			return 1;
		}
		break;
	}
	default:
		break;
	}

	return 0;*/
}


size_t Flx_Terminal::fromChild(const int32_t* data, size_t size) {
	te_process_input(_impl->term->_te, data, size);
	return size;
}

void Flx_Terminal::setToChildCB(void (*func)(const int32_t* data, size_t size, void* priv), void* priv) {
	_impl->_to_child_cb = func;
	_impl->_to_child_cb_data = priv;
}
