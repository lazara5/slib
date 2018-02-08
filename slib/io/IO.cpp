#include "slib/io/IO.h"
#include "slib/util/StringUtils.h"

namespace slib {

ByteBuffer::ByteBuffer(size_t size) {
	_buffer = (uint8_t*)malloc(size);
	_size = size;
	_length = 0;
}

ByteBuffer::~ByteBuffer() {
	if (_buffer)
		free(_buffer);
}

} // namespace slib
