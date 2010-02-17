/*
 * Flx_IResizableParent.hpp
 *
 *  Created on: Feb 18, 2010
 *      Author: jakob
 */

#ifndef FLX_IRESIZABLEPARENT_HPP_
#define FLX_IRESIZABLEPARENT_HPP_

namespace Flx {

	// Interface defining event handling for higher GUI functions.
	class IResizableParent {
	public:
		virtual void event_size_range(int minw, int minh, int maxw, int maxh, int stepw, int steph) = 0;
		virtual void event_want_size(int width, int height) = 0;
		virtual void event_title(const int32_t* text, int len) = 0;
	};

}	// namespace Flx

#endif // FLX_IRESIZABLEPARENT_HPP_
