#ifndef H_SLIB_IO_OUTPUTSTREAM_H
#define H_SLIB_IO_OUTPUTSTREAM_H

#include <stddef.h>

namespace slib {

class OutputStream {
public:
	virtual ~OutputStream();

	virtual void write(unsigned char *buffer, size_t length) = 0;
	virtual void close() = 0;
};

} // namespace slib

#endif // H_SLIB_IO_OUTPUTSTREAM_H
