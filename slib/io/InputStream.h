#ifndef H_SLIB_IO_INPUTSTREAM_H
#define H_SLIB_IO_INPUTSTREAM_H

#include "slib/io/IO.h"

namespace slib {

class InputStream {
public:
	virtual ~InputStream();

	/**
	 * Reads the next byte of data from the input stream. The value is returned as an int in the range 0 to 255.
	 * If no byte is available because the end of the stream has been reached, -1 is returned.
	 * This method blocks until input data is available, the end of the stream is detected, or an exception is thrown.
	 *
	 * @throws IOException */
	virtual int read() = 0;

	/**
	 * Reads some number of bytes from the input stream and stores them into the byte buffer b.
	 * The number of bytes actually read is returned as an integer. This method blocks until input data is available,
	 * EOF is detected, or an exception is thrown.
	 * If the available space of b is zero, then no bytes are read and 0 is returned; otherwise, there is an attempt
	 * to read at least one byte. If no byte is available because the stream is at the end of the file,
	 * the value -1 is returned; otherwise, at least one byte is read and stored into b.
	 *
	 * @param buffer  byte buffer
	 * @param length  available space in buffer
	 * @throws IOException */
	virtual ptrdiff_t read(unsigned char *buffer, size_t length) = 0;

	ptrdiff_t read(ByteBuffer &b);

	virtual void close() = 0;
};

} // namespace slib

#endif // H_SLIB_IO_INPUTSTREAM_H
