/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_FILEUTILS_H
#define H_SLIB_UTIL_FILEUTILS_H

#include "slib/exception/Exception.h"
#include "slib/lang/String.h"

#include <string>

namespace slib {

class InvalidPathException : public Exception {
public:
	InvalidPathException(const char *where, const char *msg)
	:Exception(where, "InvalidPathException", msg) {
	}
};

extern mode_t parseModeSpec(String const& modeSpec);

class FileUtils {
protected:
	/**
	 * Performs normalization on the file name
	 * @param fileName  file name
	 * @param sep  separator character
	 * @param keepSep  set to true to keep the final separator
	 * @return the normalized file name
	 */
	static std::string doNormalize(const std::string& fileName, char sep, bool keepSep);
public:
	static const char SYSTEM_SEPARATOR;

	static int mkdirs(String const& path, String const& mode = "rwxrwxr---");
	static bool isPathAbsolute(slib::String const& path);
	static UPtr<String> buildPath(String const& dir, String const& name);
	static UPtr<String> getPath(String const& fileName);

	static std::string getExtension(const std::string& fileName);

	/** @throws InvalidPathException */
	static std::string normalize(const std::string& fileName);
};

} // namespace slib

#endif // H_SLIB_UTIL_FILEUTILS_H
