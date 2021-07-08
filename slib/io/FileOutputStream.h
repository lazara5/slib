#ifndef H_SLIB_IO_FILEOUTPUTSTREAM_H
#define H_SLIB_IO_FILEOUTPUTSTREAM_H

#include "slib/io/OutputStream.h"

#include <stdio.h>

#include <string>

namespace slib {

class FileOutputStream : public OutputStream {
private:
	FILE *_f;
public:
	FileOutputStream(std::string const& fileName, bool append = false);
	virtual ~FileOutputStream();

	virtual void write(unsigned char *buffer, size_t length) override;
	virtual void close() override;
};

} // namespace slib

#endif // H_SLIB_IO_FILEOUTPUTSTREAM_H
