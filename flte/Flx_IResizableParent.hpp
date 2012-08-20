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
		/**
		 * notify parent widget of resize constraints of child widget
		 * requested width constrant is: w = minw + n*stepw, where n is an integer such that w<=maxw
		 * requested height constraint is: h = minh + m*steph, where m is an integer such that h<=maxh
		 *
		 * \param minw	minimum width
		 * \param minh	minimum height
		 * \param maxw	maximum width
		 * \param maxh	maximum height
		 * \param stepw	width size increments (over minw)
		 * \param steph height size increments (over minh)width should be of form minw+n*stepw
		 */
		virtual void event_size_range(int minw, int minh, int maxw, int maxh, int stepw, int steph) = 0;
		
		/**
		 * request from parent, a specific size for this child widget
		 * \param width		desired width
		 * \param height	desired height
		 */
		virtual void event_want_size(int width, int height) = 0;
		
		/**
		 * request a new title for this widget (TODO: should probably go into ScrolledTerm or such)
		 */
		virtual void event_title(const int32_t* text, int len) = 0;
		
		virtual void event_childexit(int exit_status) = 0;
	};

}	// namespace Flx

#endif // FLX_IRESIZABLEPARENT_HPP_
