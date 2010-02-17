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
#include <assert.h>

#include "Flxi_BasicTerm.hpp"

#include "Flx_ScrolledTerm.hpp"

namespace Flx {
namespace VT {


#define SCROLLBAR_WIDTH		12

typedef unsigned int uint;

static double dbl_max(double a, double b) {
	return (a > b) ? a : b;
}

namespace impl {

class ScrolledTermPriv : public BasicTerm::IEventHandler {
public:
	friend class ScrolledTerm;

	ScrolledTerm* _p;

	IResizableParent* _parenth;

	BasicTerm*		term;
	Fl_Scrollbar*	scrollbar;
	bool			scroll_lock;

public:

	ScrolledTermPriv(ScrolledTerm* p) {
		_p = p;
	}

	void _widget_cb(Fl_Widget* widget) {
		if (widget == term) {
			printf( "term clicked!\n" );
		} else if (widget == scrollbar) {
			printf( "scrollbar: %d\n", scrollbar->value());
			scrollbar->redraw();
			term->tePosition(scrollbar->value());
		}
	}

	// Called by FLTK widget actions ('priv' is 'this')
	static void _widget_cb(Fl_Widget* widget, void* priv) {
		((ScrolledTermPriv*)priv)->_widget_cb(widget);
	}


private:
	virtual void event_scrollposition(int offset, int size) {
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

	virtual void event_size_range(int minw, int minh, int maxw, int maxh, int stepw, int steph) {
		const int dw = Fl::box_dw(_p->box()) + SCROLLBAR_WIDTH;
		const int dh = Fl::box_dh(_p->box());

		minw += dw;
		maxw += dw;
		minh += dh;
		maxh += dh;

		_parenth->event_size_range(minw, minh, maxw, maxh, stepw, steph);
	}

	virtual void event_want_size(int width, int height) {
		const int dw = Fl::box_dw(_p->box()) + SCROLLBAR_WIDTH;
		const int dh = Fl::box_dh(_p->box());

		width += dw;
		height += dh;

		printf("cock %d bapp: %d\n", height, scrollbar->h());
		//assert (height >= scrollbar->h());

		_parenth->event_want_size(width, height);
	}

	virtual void event_title(const int32_t* text, int len) {
		_parenth->event_title(text, len);
	}
};


}

ScrolledTerm::ScrolledTerm(		IResizableParent* parenth, IChildHandler* childh,
								int X, int Y, int W, int H
	)
// Parent constructors
	: Fl_Group(X,Y,W,H)
	, _impl(new impl::ScrolledTermPriv(this))
{

	// inner dimensions
	const uint iw = W - Fl::box_dw(box());
	const uint ih = H - Fl::box_dh(box());

	uint x = X + Fl::box_dx(box());
	uint y = Y + Fl::box_dy(box());

	_impl->_parenth = parenth;

	_impl->term = new impl::BasicTerm(14, childh, _impl, x, y, iw-SCROLLBAR_WIDTH, ih);
	x += iw-SCROLLBAR_WIDTH;

	_impl->scrollbar = new Fl_Scrollbar(x, y, SCROLLBAR_WIDTH, ih);
	_impl->scrollbar->step(1);
	_impl->scrollbar->callback(& (impl::ScrolledTermPriv::_widget_cb), _impl);

	end();

	_impl->scroll_lock = false;

	resizable(_impl->term);
}

ScrolledTerm::~ScrolledTerm() {
	delete _impl->scrollbar;
	delete _impl->term;
	delete _impl;
}

void ScrolledTerm::init() {
	_impl->term->init();
}

int ScrolledTerm::handle(int event) {
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
#ifndef NDEBUG
		case 'd':
			if (shift && Fl::event_ctrl()) {
				printf("forced debug!\n");
				_impl->term->printTerminalDebug(stdout);
				return 1;
			}
			break;
#endif
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
void ScrolledTerm::scrollReset(void) {
	_impl->term->tePosition(0);
}

void ScrolledTerm::scrollUp(void) {
	const int h = _impl->term->teGetHeight();
	_impl->term->tePosition(_impl->scrollbar->value() + (2*h)/3);
}

void ScrolledTerm::scrollDown(void) {
	const int h = _impl->term->teGetHeight();
	_impl->term->tePosition(_impl->scrollbar->value() - (2*h)/3);
}

// Scroll-locking
bool ScrolledTerm::getScrollLock() {
	return _impl->scroll_lock;
}

void ScrolledTerm::setScrollLock(bool lock) {
	_impl->term->teLockScroll(lock);
}


size_t ScrolledTerm::fromChild(const int32_t* data, size_t size) {
	_impl->term->teProcessInput(data, size);
	return size;
}

}	// namespace Flx
}	// namespace Terminal
