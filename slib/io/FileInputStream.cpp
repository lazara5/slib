#include "slib/io/FileInputStream.h"
#include "slib/util/StringUtils.h"

namespace slib {

FileInputStream::FileInputStream(std::string const& fileName) {
	_f = fopen(fileName.c_str(), "r");
	if (!_f)
		throw FileNotFoundException(_HERE_, fmt::format("fopen() failed, errno='{}'", StringUtils::formatErrno()).c_str());
}

FileInputStream::~FileInputStream() {
	if (_f)
		fclose(_f);
}

int FileInputStream::read() {
	if (!_f)
		throw IOException(_HERE_, "Stream closed");
	int c = fgetc(_f);
	if (c != EOF)
		return c;
	if (ferror(_f))
		throw IOException(_HERE_, fmt::format("File I/O error, errno='{}'", StringUtils::formatErrno()).c_str());
	else
		return -1;
}

ptrdiff_t FileInputStream::read(unsigned char *buffer, size_t length) {
	if (!_f)
		throw IOException(_HERE_, "Stream closed");

	if (length == 0)
		return 0;

	size_t nRead = fread(buffer, 1, length, _f);
	if (nRead < length) {
		if (ferror(_f))
			throw IOException(_HERE_, fmt::format("File I/O error, errno='{}'", StringUtils::formatErrno()).c_str());
		if ((nRead == 0) && feof(_f))
			return -1;
	}
	return (ptrdiff_t)nRead;
}

void FileInputStream::close() {
	if (_f)
		fclose(_f);
	_f = nullptr;
}

} // namespace slib

