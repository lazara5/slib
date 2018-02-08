#include "slib/io/FileOutputStream.h"
#include "slib/util/StringUtils.h"

namespace slib {

FileOutputStream::FileOutputStream(const std::string &fileName, bool append /* = false */) {
	_f = fopen(fileName.c_str(), append ? "a" : "w");
	if (!_f)
		throw FileNotFoundException(_HERE_, fmt::format("fopen() failed, errno='{}'", StringUtils::formatErrno()).c_str());
}

FileOutputStream::~FileOutputStream() {
	if (_f)
		fclose(_f);
}

void FileOutputStream::write(unsigned char *buffer, size_t length) {
	if (!_f)
		throw IOException(_HERE_, "Stream closed");
	size_t nWritten = fwrite(buffer, length, 1, _f);
	if (nWritten != 1)
		throw IOException(_HERE_, fmt::format("File I/O error, errno='{}'", StringUtils::formatErrno()).c_str());
}

void FileOutputStream::close() {
	if (_f)
		fclose(_f);
	_f = nullptr;
}


} // namespace slib

