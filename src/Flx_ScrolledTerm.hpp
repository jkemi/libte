/*
 * Flx_Terminal.hpp
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#ifndef FLX_SCROLLEDTERM_HPP_
#define FLX_SCROLLEDTERM_HPP_

#include <FL/Fl_Group.H>

#include <stdint.h>		// for int32_t
#include <stddef.h>		// for size_t

#include "Flx_IResizableParent.hpp"
#include "Flx_IChildHandler.hpp"

namespace Flx {
namespace VT {

namespace impl {
	class ScrolledTermPriv;
}

class ScrolledTerm : public Fl_Group {
private:
	impl::ScrolledTermPriv* const _impl;
public:
	ScrolledTerm(IResizableParent* parenth, IChildHandler* childh, int X, int Y, int W, int H);
	virtual ~ScrolledTerm();

	virtual void init();

	virtual int handle(int event);

	// Scrolling
	void scrollReset(void);
	void scrollUp(void);
	void scrollDown(void);

	// Scroll-locking
	bool getScrollLock();
	void setScrollLock(bool lock);

	// Terminal I/O
	size_t	fromChild(const int32_t* data, size_t size);
};

}	// namespace VT
}	// namespace Flx

#endif // FLX_SCROLLEDTERM_HPP_
