/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_FILEUTILS_H
#define H_SLIB_UTIL_FILEUTILS_H

#include "slib/exception/Exception.h"
#include "slib/lang/String.h"
#include "slib/util/StringUtils.h"
#include "slib/io/IO.h"
#include "slib/io/FileInputStream.h"

#include <string>

#include <sys/stat.h>
#include <sys/types.h>

namespace slib {

class FileUtils {
private:
	static bool isMode(char c, char setValue) {
		if (c == setValue)
			return true;
		if (c == '-')
			return false;
		throw IllegalArgumentException(_HERE_, "Invalid mode");
	}

public:

	template <class S>
	static mode_t parseModeSpec(S const* modeSpec) {
		size_t modeLen = strLen(modeSpec);
		if ((modeLen < 9) || (modeLen > 10))
			throw (int)EINVAL;

		const char *modeStr = strData(modeSpec);
		mode_t mode = 0;
		try {
			if (isMode(modeStr[0], 'r')) mode |= S_IRUSR;
			if (isMode(modeStr[1], 'w')) mode |= S_IWUSR;
			if (isMode(modeStr[2], 'x')) mode |= S_IXUSR;
			if (isMode(modeStr[3], 'r')) mode |= S_IRGRP;
			if (isMode(modeStr[4], 'w')) mode |= S_IWGRP;
			if (isMode(modeStr[5], 'x')) mode |= S_IXGRP;
			if (isMode(modeStr[6], 'r')) mode |= S_IROTH;
			if (isMode(modeStr[7], 'w')) mode |= S_IWOTH;
			if (isMode(modeStr[8], 'x')) mode |= S_IXOTH;
			if (modeLen > 9) {
				if (isMode(modeStr[9], 't')) mode |= S_ISVTX;
			}
		} catch (IllegalArgumentException const&) {
			throw (int)EINVAL;
		}

		return mode;
	}

	template <class S1, class S2>
	static int mkdirs(S1 const* path, S2 const* modeSpec = "rwxrwxr---") {
		mode_t mode = 0;
		try {
			mode = parseModeSpec(modeSpec);
		} catch (int err) {
			errno = err;
			return -1;
		}

		char *tmpPath = strdup(cStr(path));
		char *p = tmpPath;

		while (*p != '\0') {
			// skip first character in case it is /
			p++;

			while ((*p != '\0') && (*p != '/'))
				p++;

			char v = *p;
			*p = '\0';

			if ((mkdir(tmpPath, mode) == -1) && (errno != EEXIST)) {
				free(tmpPath);
				return -1;
			}

			*p = v;
		}

		free(tmpPath);
		return 0;
	}

	template <class S>
	static size_t getSize(S const* fileName) {
		struct stat st;
		int ret = stat(cStr(fileName), &st);
		if (ret != 0)
			throw IOException(_HERE_, fmt::format("File I/O error, errno='{}'", StringUtils::formatErrno()).c_str());
		return st.st_size;
	}

	template <class S>
	static UPtr<ByteBuffer> readAllBytes(S const* fileName) {
		size_t size = getSize(fileName);

		UPtr<ByteBuffer> data = newU<ByteBuffer>(size);
		FileInputStream in(fileName);
		ptrdiff_t read = in.read(data->getBuffer(), size);
		if (read >= 0)
			data->setLength(read);
		return data;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_FILEUTILS_H
