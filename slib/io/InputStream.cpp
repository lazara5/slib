#include "slib/io/InputStream.h"

namespace slib {

InputStream::~InputStream() {}

ptrdiff_t InputStream::read(ByteBuffer &b) {
	ptrdiff_t nRead = read(b.getFreePtr(), b.getFreeLength());
	if (nRead > 0)
		b.setLength(b.getLength() + (size_t)nRead);
	return nRead;
}

} // namespace slib
