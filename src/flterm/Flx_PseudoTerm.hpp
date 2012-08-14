/*
 * Flx_Terminal.hpp
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#ifndef FLX_PSEUDOTERM_HPP_
#define FLX_PSEUDOTERM_HPP_

#include "Flx_ScrolledTerm.hpp"

#include "pty/pty.h"

namespace Flx {
namespace VT {

namespace impl {
	class PseudoTermPriv;
}

class PseudoTerm : public ScrolledTerm,  protected Flx::VT::IChildHandler {
private:
	impl::PseudoTermPriv* const _impl;
public:
	PseudoTerm(IResizableParent* resizable, PTY* pty, int X, int Y, int W, int H);
	virtual ~PseudoTerm();

	// Terminal I/O
private:
	size_t	fromChild(const int32_t* data, size_t size);

protected:
	// IChildHandler interface
	void child_sendto(const int32_t* data, int len);
	bool child_resize(int width, int height);

};

}	// namespace VT
}	// namespace Flx

#endif // FLX_PSEUDOTERM_HPP_
