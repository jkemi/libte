/*
 * Flx_Terminal.hpp
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#ifndef FLX_TERMINAL_HPP_
#define FLX_TERMINAL_HPP_

#include <FL/Fl_Group.H>

#include <stdint.h>		// for int32_t
#include <stddef.h>		// for size_t

class Flx_Terminal_Impl;

class Flx_Terminal : public Fl_Group {
private:
	Flx_Terminal_Impl* 	_impl;
public:
	Flx_Terminal(int X, int Y, int W, int H, const char* label);
	virtual ~Flx_Terminal();

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
	void	setToChildCB(void (*func)(const int32_t* data, size_t size, void* priv), void* priv);
	void	setTermSizeCB(void (*func)(int width, int height, void* priv), void* priv);
};


#endif /* FLX_TERMINAL_HPP_ */
