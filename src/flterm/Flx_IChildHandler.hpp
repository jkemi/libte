#ifndef FLX_CHILDHANDLER_HPP_
#define FLX_CHILDHANDLER_HPP_

namespace Flx {
namespace VT {

// Interface defining event handling for slave tty process.
class IChildHandler {
public:
	virtual void child_resize(int width, int height) = 0;
	virtual void child_sendto(const int32_t* data, int len) = 0;
};

}	// namespace VT
}	// namespace Flx

#endif // FLX_CHILDHANDLER_HPP_
