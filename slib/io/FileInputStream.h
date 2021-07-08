#ifndef H_SLIB_IO_FILEINPUTSTREAM_H
#define H_SLIB_IO_FILEINPUTSTREAM_H

#include "slib/io/InputStream.h"
#include "slib/exception/IOException.h"
#include "slib/lang/String.h"
#include "slib/util/StringUtils.h"

namespace slib {

class FileInputStream : public InputStream {
private:
	FILE *_f;
public:
	template <class S>
	FileInputStream(S const& fileName) {
		_f = fopen(cStr(CPtr(fileName)), "r");
		if (!_f)
			throw FileNotFoundException(_HERE_, fmt::format("fopen() failed, errno='{}'", StringUtils::formatErrno()).c_str());
	}

	virtual ~FileInputStream() override;

	/** @throws IOException */
	virtual int read() override;

	/** @throws IOException */
	virtual ssize_t read(uint8_t *buffer, size_t length) override;

	virtual void close() override;
};

} // namespace slib

#endif
