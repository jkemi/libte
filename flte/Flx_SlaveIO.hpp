/*
 * Flx_SlaveIO.hpp
 *
 *  Created on: Aug 23, 2008
 *      Author: jakob
 */

#ifndef FLX_SLAVEIO_HPP_
#define FLX_SLAVEIO_HPP_

namespace Flx {
namespace VT {

/**
 * Abstract interface defining I/O interactions with slave.
 * Typically the slave is a child terminal application process, but could also be something else.
 * Implementations are responsible for pumping data from slave to fromSlave() method.
 */
class SlaveIO {

	/**
	 * Handler for data sent from slave.
	 *
	 * \param data		pointer to data.
	 * \param len		length of child data, or 0 for exit in which case data[0] contains exit status
	 * \param priv		private handler data
	 */
	typedef void (*handler_t)(const int32_t* data, size_t len, void* handler_priv);
	
private:
	handler_t	_handler;
	void*		_handler_priv;
	
public:
	
	SlaveIO() {
		_handler = NULL;
		_handler_priv = NULL;
	}
	
	/**
	 * Assign handler callback of data sent from slave.
	 *
	 * \param handler		handler callback
	 * \param handler_priv	private handler data
	 */
	void setHandler( handler_t handler, void* handler_priv) {
		_handler = handler;
		_handler_priv = handler_priv;
	}
	
	/**
	 * Request resize of slave.
	 *
	 * \param width		new width in columns
	 * \param height	new height in rows
	 * \return true if slave could be resized
	 */
	virtual bool resizeSlave(int width, int height) = 0;
	
	/**
	 * Send data to slave.
	 *
	 * \param data	pointer to ucs4 data
	 * \param len	number of ucs4 codepoints
	 * \return false on I/O failure
	 */
	virtual bool toSlave(const int32_t* data, int len) = 0;
	
protected:
	
	/**
	 * Subclasses should use this method to pass data to assigned handler
	 */
	void fromSlave(const int32_t* data, size_t len) {
		if (_handler != NULL) {
			_handler(data, len, _handler_priv);
		}
	}
};

/**
 * Implements SlaveIO for pty slave process.
 */
class PtyIO : public SlaveIO {
private:
	static const size_t _BUFSIZE = 1024;
	
private:
	
	const int 	_fd;
	char		_buf[_BUFSIZE];
	size_t		_fill;
	
	// Called by FLTK whenever input is received on filedes
	static void _s_fd_cb(int fd, void* priv) {
		((PtyIO*)priv)->_fromSlave(fd);
	}
	
public:
	PtyIO(int fd);
	~PtyIO();
	bool resizeSlave(int width, int height);
	bool toSlave(const int32_t* data, int len);
	
private:
	// Called by FLTK whenever input is received on filedes
	void _fromSlave(int fd);
};


}	// namespace VT
}	// namespace Flx

#endif // FLX_SLAVEIO_HPP_
