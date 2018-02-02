/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_FILEUTILS_H
#define H_SLIB_UTIL_FILEUTILS_H

#include "slib/exception/Exception.h"

#include <string>

namespace slib {

class InvalidPathException : public Exception {
public:
	InvalidPathException(const char *where, const char *msg)
	:Exception(where, "InvalidPathException", msg) {
	}
};

extern mode_t parseModeSpec(const std::string& modeSpec);

class FileUtils {
protected:
	static std::string doNormalize(const std::string& fileName, char sep, bool keepSep);
public:
	static const char SYSTEM_SEPARATOR;

	static int mkdirs(const std::string& path, const std::string& mode = "rwxrwxr---");
	static bool isPathAbsolute(const std::string& path);
	static std::string buildPath(const std::string& dir, const std::string& name);
	static std::string getPath(const std::string& fileName);

	static std::string getExtension(const std::string& fileName);

	/** @throws InvalidPathException */
	static std::string normalize(const std::string& fileName);
};

} // namespace slib

#endif // H_SLIB_UTIL_FILEUTILS_H
