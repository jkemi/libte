/*
 * Flx_Terminal.cxx
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scrollbar.H>

#include <stdio.h>

#include "Fl_Term.h"

#include "Flx_Terminal.hpp"

typedef unsigned int uint;

#define SCROLLBAR_WIDTH		12

static double dbl_min(double a, double b) {
	return (a < b) ? a : b;
}

static double dbl_max(double a, double b) {
	return (a > b) ? a : b;
}

class Flx_Terminal_Impl {
public:
	void (*_to_child_cb)(const int32_t* data, size_t size, void* priv);
	void* _to_child_cb_data;
	void (*_term_size_cb)(int width, int height, void* priv);
	void* _term_size_cb_data;

	Fl_Term*		term;
	Fl_Scrollbar*	scrollbar;
	bool			scroll_lock;

public:
	// Called by FLTK widget actions
	void _widget_cb(Fl_Widget* widget) {
		if (widget == term) {
			printf( "term clicked!\n" );
		} else if (widget == scrollbar) {
			printf( "scrollbar: %d\n", scrollbar->value());
			scrollbar->redraw();
			term->tePosition(scrollbar->value());
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
			scrollbar->slider_size( dbl_max(1.0/(size+1), 1.0/20.0) );
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

	void termSize(int width, int height) {

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

// Called by Fl_Term on terminal resize
static void _term_size_cb(void* priv, int width, int height) {
	Flx_Terminal_Impl* impl = (Flx_Terminal_Impl*)priv;

	if (impl->_term_size_cb != 0) {
		impl->_term_size_cb(width, height, impl->_term_size_cb_data);
	}
}

Flx_Terminal::Flx_Terminal(int X, int Y, int W, int H, const char* label) : Fl_Group(X,Y,W,H) {
	_impl = new Flx_Terminal_Impl();

	// inner dimensions
	const uint iw = W - Fl::box_dw(box());
	const uint ih = H - Fl::box_dh(box());

	uint x = X + Fl::box_dx(box());
	uint y = Y + Fl::box_dy(box());


	_impl->term = new Fl_Term(14, x, y, iw-SCROLLBAR_WIDTH, ih);
	x += iw-SCROLLBAR_WIDTH;

	_impl->scrollbar = new Fl_Scrollbar(x, y, SCROLLBAR_WIDTH, ih);
	_impl->scrollbar->step(1);
	_impl->scrollbar->callback(&_widget_cb, _impl);

	end();

	_impl->term->set_scroll_func(&_scroll_cb, _impl);
	_impl->term->set_send_back_func(&_send_back_cb, _impl);
	_impl->term->set_size_func(_term_size_cb, _impl);

	_impl->_scroll_cb(0, 0);

	_impl->scroll_lock = false;

	resizable(_impl->term);
}

Flx_Terminal::~Flx_Terminal() {
	delete _impl->scrollbar;
	delete _impl->term;
	delete _impl;
}

int Flx_Terminal::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	case FL_KEYBOARD: {
		const bool shift = (Fl::event_shift() != 0);
		switch (Fl::event_key()) {
		case FL_Page_Up:
			if (shift) {
				printf ("page up!\n");
				scrollUp();
				return 1;
			}
			break;
		case FL_Page_Down:
			if (shift) {
				printf ("page down!\n");
				scrollDown();
				return 1;
			}
			break;
		case FL_Scroll_Lock: {
			const bool scroll_lock = !getScrollLock();
			printf("scroll lock: %d\n", scroll_lock);
			setScrollLock(scroll_lock);
			return 1;
			break;
		}
		case 'r':
			if (shift && Fl::event_ctrl()) {
				printf("forced redraw!\n");
				_impl->term->redraw();
				return 1;
			}
			break;
		}
		return _impl->term->handle(event);
		break;
	}
	default:
		break;
	}

	return Fl_Group::handle(event);
}

// Scrolling
void Flx_Terminal::scrollReset(void) {
	_impl->term->tePosition(0);
}

void Flx_Terminal::scrollUp(void) {
	const int h = _impl->term->teGetHeight();
	_impl->term->tePosition(_impl->scrollbar->value() + (2*h)/3);
}

void Flx_Terminal::scrollDown(void) {
	const int h = _impl->term->teGetHeight();
	_impl->term->tePosition(_impl->scrollbar->value() - (2*h)/3);
}

// Scroll-locking
bool Flx_Terminal::getScrollLock() {
	return _impl->scroll_lock;
}

void Flx_Terminal::setScrollLock(bool lock) {
	_impl->term->teLockScroll(lock);
}


size_t Flx_Terminal::fromChild(const int32_t* data, size_t size) {
	_impl->term->teProcessInput(data, size);
	return size;
}

void Flx_Terminal::setToChildCB(void (*func)(const int32_t* data, size_t size, void* priv), void* priv) {
	_impl->_to_child_cb = func;
	_impl->_to_child_cb_data = priv;
}

void Flx_Terminal::setTermSizeCB(void (*func)(int width, int height, void* priv), void* priv) {
	_impl->_term_size_cb = func;
	_impl->_term_size_cb_data = priv;
	if (func != NULL) {
		func(_impl->term->teGetWidth(), _impl->term->teGetHeight(), priv);
	}
}
