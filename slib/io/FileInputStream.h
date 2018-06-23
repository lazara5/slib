#ifndef H_SLIB_IO_FILEINPUTSTREAM_H
#define H_SLIB_IO_FILEINPUTSTREAM_H

#include "slib/io/InputStream.h"
#include "slib/lang/String.h"

namespace slib {

class FileInputStream : public InputStream {
private:
	FILE *_f;
public:
	FileInputStream(String const& fileName);
	virtual ~FileInputStream() override;

	/** @throws IOException */
	virtual int read() override;

	/** @throws IOException */
	virtual ptrdiff_t read(unsigned char *buffer, size_t length) override;

	virtual void close() override;
};

} // namespace slib

#endif
