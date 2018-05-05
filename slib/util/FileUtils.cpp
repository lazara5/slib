/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/FileUtils.h"
#include "slib/String.h"
#include "slib/exception/IllegalArgumentException.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include "fmt/format.h"

#include <algorithm>
#include <vector>

namespace slib {

/** The Unix separator character */
const char UNIX_SEPARATOR = '/';

/** The Windows separator character */
const char WINDOWS_SEPARATOR = '\\';

const char FileUtils::SYSTEM_SEPARATOR = UNIX_SEPARATOR;
const char OTHER_SEPARATOR = WINDOWS_SEPARATOR;

bool isMode(char c, char setValue) {
	if (c == setValue)
		return true;
	if (c == '-')
		return false;
	throw IllegalArgumentException(_HERE_, "Invalid mode");
}

mode_t parseModeSpec(const std::string& modeSpec) {
	size_t modeLen = modeSpec.size();
	if ((modeLen < 9) || (modeLen > 10))
		throw (int)EINVAL;

	const char *modeStr = modeSpec.c_str();
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

int FileUtils::mkdirs(const std::string& path, const std::string& modeSpec) {
	mode_t mode = 0;
	try {
		mode = parseModeSpec(modeSpec);
	} catch (int err) {
		errno = err;
		return -1;
	}

	char *tmpPath = strdup(path.c_str());
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

bool FileUtils::isPathAbsolute(const std::string& path) {
	if (path.empty())
		return false;
	if (String::startsWith(Ptr(path), '/'))
		return true;
	return false;
}

std::string FileUtils::buildPath(const std::string& dir, const std::string& name) {
	std::string path = name;
	if (dir.empty())
		return name;
	if (!isPathAbsolute(name)) {
		size_t l = dir.size();
		if (l > 0 && String::endsWith(Ptr(dir), '/'))
			path = dir + name;
		else
			path = dir + '/' + name;
	}
	return path;
}

std::string FileUtils::getPath(const std::string& fileName) {
	std::ptrdiff_t lastSep = String::lastIndexOf(Ptr(fileName), '/');
	if (lastSep < 0)
		return fileName;
	return String::substring(Ptr(fileName), 0, (size_t)lastSep);
}

ptrdiff_t indexOfLastSep(const std::string& fileName) {
	return String::lastIndexOf(Ptr(fileName), '/');
}

ptrdiff_t indexOfExtSep(const std::string& fileName) {
	ptrdiff_t extPos = String::lastIndexOf(Ptr(fileName), '.');
	ptrdiff_t lastSep = indexOfLastSep(fileName);
	if (lastSep > extPos)
		return -1;
	return extPos;
}

std::string FileUtils::getExtension(const std::string& fileName) {
	ptrdiff_t index = indexOfExtSep(fileName);
	if (index < 0)
		return "";
	return String::substring(Ptr(fileName), (size_t)index + 1);
}

bool isSep(char ch) {
	return ch == UNIX_SEPARATOR || ch == WINDOWS_SEPARATOR;
}

/**
 * Returns the length of the filename prefix, such as <code>C:/</code> or <code>~/</code>.
 * This method will handle a file name in either Unix or Windows format.
 * The prefix length includes the first slash in the full file name
 * if applicable. Thus, it is possible that the length returned is greater
 * than the length of the input string.
 *
 * Windows:
 * a\b\c.txt           --&gt; ""          --&gt; relative
 * \a\b\c.txt          --&gt; "\"         --&gt; current drive absolute
 * C:a\b\c.txt         --&gt; "C:"        --&gt; drive relative
 * C:\a\b\c.txt        --&gt; "C:\"       --&gt; absolute
 * \\server\a\b\c.txt  --&gt; "\\server\" --&gt; UNC
 * \\\a\b\c.txt        --&gt;  error, length = -1
 *
 * Unix:
 * a/b/c.txt           --&gt; ""          --&gt; relative
 * /a/b/c.txt          --&gt; "/"         --&gt; absolute
 * ~/a/b/c.txt         --&gt; "~/"        --&gt; current user
 * ~                   --&gt; "~/"        --&gt; current user (slash added)
 * ~user/a/b/c.txt     --&gt; "~user/"    --&gt; named user
 * ~user               --&gt; "~user/"    --&gt; named user (slash added)
 * //server/a/b/c.txt  --&gt; "//server/"
 * ///a/b/c.txt        --&gt; error, length = -1
 *
 * Note that a leading // (or \\) is used to indicate a UNC name on Windows.
 * These must be followed by a server name, so double-slashes are not collapsed
 * to a single slash at the start of the filename.
 *
 * @param fileName  the filename to find the prefix in
 * @return the length of the prefix, -1 if invalid
 */
ptrdiff_t getPrefixLength(const std::string& fileName) {
	size_t len = fileName.size();
	if (len == 0)
		return 0;
	
	char c0 = fileName[0];

	if (len == 1) {
		if (c0 == '~')
			return 2;  // return a length greater than the input
		return isSep(c0) ? 1 : 0;
	} else {
		if (c0 == '~') {
			ptrdiff_t posUnix = String::indexOf(Ptr(fileName), UNIX_SEPARATOR, 1);
			ptrdiff_t posWin = String::indexOf(Ptr(fileName), WINDOWS_SEPARATOR, 1);
			if (posUnix < 0 && posWin < 0)
				return (ptrdiff_t)len + 1;  // return a length greater than the input

			posUnix = posUnix < 0 ? posWin : posUnix;
			posWin = posWin < 0 ? posUnix : posWin;
			return std::min(posUnix, posWin) + 1;
		}

		char c1 = fileName[1];
		if (c1 == ':') {
			c0 = (char)toupper(c0);
			if (c0 >= 'A' && c0 <= 'Z') {
				if (len == 2 || isSep(fileName[2]) == false)
					return 2;
				return 3;
			} else if (c0 == UNIX_SEPARATOR)
				return 1;
			return -1;
		} else if (isSep(c0) && isSep(c1)) {
			ptrdiff_t posUnix = String::indexOf(Ptr(fileName), UNIX_SEPARATOR, 2);
			ptrdiff_t posWin = String::indexOf(Ptr(fileName), WINDOWS_SEPARATOR, 2);
			if ((posUnix == -1 && posWin == -1) || posUnix == 2 || posWin == 2)
				return -1;
			posUnix = posUnix < 0 ? posWin : posUnix;
			posWin = posWin < 0 ? posUnix : posWin;
			return std::min(posUnix, posWin) + 1;
		} else
			return isSep(c0) ? 1 : 0;
	}
}

std::string FileUtils::doNormalize(const std::string& fileName, char sep, bool keepSep) {
	size_t size = fileName.size();
	if (size == 0)
		return fileName;

	ptrdiff_t prefixLen = getPrefixLength(fileName);
	if (prefixLen < 0)
		throw InvalidPathException(_HERE_, fmt::format("Invalid path: '{}'", fileName).c_str());
	size_t prefix = (size_t)prefixLen;

	std::vector<char> array(size + 2);
	memcpy(array.data(), fileName.c_str(), fileName.size() * sizeof(char));

	// fix separators throughout
	char otherSep = (sep == SYSTEM_SEPARATOR) ? OTHER_SEPARATOR : SYSTEM_SEPARATOR;
	for (unsigned int i = 0; i < array.size(); i++) {
		if (array[i] == otherSep)
			array[i] = sep;
	}

	// add extra separator on the end to simplify code below
	bool lastIsDir = true;
	if (array[size - 1] != sep) {
		array[size++] = sep;
		lastIsDir = false;
	}

	// //
	for (size_t i = prefix + 1; i < size; i++) {
		if (array[i] == sep && array[i - 1] == sep) {
			array.erase(array.begin() + (ptrdiff_t)i - 1);
			size--;
			i--;
		}
	}

	// ./
	for (size_t i = prefix + 1; i < size; i++) {
		if (array[i] == sep && array[i - 1] == '.' && (i == prefix + 1 || array[i - 2] == sep)) {
			if (i == size - 1)
				lastIsDir = true;
			array.erase(array.begin() + (ptrdiff_t)i - 1, array.begin() + (ptrdiff_t)i + 1);
			size -= 2;
			i--;
		}
	}

	// ../
	for (size_t i = prefix + 2; i < size; i++) {
		if (array[i] == sep && array[i - 1] == '.' && array[i - 2] == '.' && (i == prefix + 2 || array[i - 3] == sep)) {
			if (i == prefix + 2)
				throw InvalidPathException(_HERE_, fmt::format("Invalid path after normalization: '{}'", fileName).c_str());

			if (i == size - 1)
				lastIsDir = true;

			size_t j;
			bool continueOuter = false;

			for (j = i - 4 ; j >= prefix; j--) {
				if (array[j] == sep) {
					// remove b/../ from a/b/../c
					array.erase(array.begin() + (ptrdiff_t)j + 1, array.begin() + (ptrdiff_t)i + 1);
					size -= i - j;
					i = j + 1;
					continueOuter = true;
					break;
				}
			}

			if (continueOuter)
				continue;

			// remove a/../ from a/../c
			array.erase(array.begin() + (ptrdiff_t)prefix, array.begin() + (ptrdiff_t)i + 1);
			size -= i + 1 - prefix;
			i = prefix + 1;
		}
	}

	if (size <= 0)  // should never be less than 0
		return "";

	if (size <= prefix) {  // should never be less than prefix
		return std::string(array.begin(), array.begin() + (ptrdiff_t)size);
	}

	if (lastIsDir && keepSep)
		return std::string(array.begin(), array.begin() + (ptrdiff_t)size); // keep trailing separator

	return std::string(array.begin(), array.begin() + (ptrdiff_t)size - 1); // drop trailing separator
}

std::string FileUtils::normalize(const std::string& fileName) {
	return doNormalize(fileName, SYSTEM_SEPARATOR, true);
}

} // namespace
