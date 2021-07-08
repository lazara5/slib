#include "slib/io/InputStream.h"

namespace slib {

InputStream::~InputStream() {}

ssize_t InputStream::read(Array<uint8_t> &b) {
	ssize_t nRead = read(b.data(), b.length());
	/*if (nRead > 0)
		b.setLength(b.getLength() + (size_t)nRead);*/
	return nRead;
}

} // namespace slib
