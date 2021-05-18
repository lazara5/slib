#ifndef H_SLIB_IO_IO_H
#define H_SLIB_IO_IO_H

#include "slib/exception/IOException.h"

#include <stddef.h>
#include <stdint.h>

namespace slib {

class ByteBuffer {
private:
	uint8_t *_buffer;
	size_t _size;
	size_t _length; // how much useful data we have
public:
	ByteBuffer(size_t size);

	virtual ~ByteBuffer();

	uint8_t *getBuffer() const {
		return _buffer;
	}

	uint8_t operator[](int i) const {
		return _buffer[i];
	}

	/**
	 * returns a pointer to the first 'free' position in the buffer
	 * (that is <buffer> + <length>)
	 */
	uint8_t *getFreePtr() const {
		return _buffer + _length;
	}

	size_t getSize() const {
		return _size;
	}

	size_t getLength() const {
		return _length;
	}

	void setLength(size_t l) {
		if (l > _size)
			throw IndexOutOfBoundsException(_HERE_);
		_length = l;
	}

	// returns the number of bytes between <length> and <size>
	size_t getFreeLength() const {
		if (_size > _length)
			return _size - _length;
		return 0;
	}

	virtual void clear() {
		setLength(0);
	}
};

} // namespace slib

#endif // H_SLIB_IO_IO_H
