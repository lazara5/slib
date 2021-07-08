/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Based on Apache Commons IO FilenameUtils
 */

#ifndef H_SLIB_UTIL_FILENAMEUTILS_H
#define H_SLIB_UTIL_FILENAMEUTILS_H

#include "slib/lang/String.h"
#include "slib/util/StringUtils.h"

#include <vector>

namespace slib {

class FilenameUtils {
private:
	static const char WINDOWS_SEPARATOR;
	static const char UNIX_SEPARATOR;

	static const char OTHER_SEPARATOR;

	static bool isSeparator(char ch) {
		return ch == UNIX_SEPARATOR || ch == WINDOWS_SEPARATOR;
	}
public:
	static const char SYSTEM_SEPARATOR;

	class InvalidPathException : public Exception {
	public:
		InvalidPathException(const char *where, const char *msg)
		:Exception(where, "InvalidPathException", msg) {
		}
	};

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
	template <class S>
	static int getPrefixLength(S const& fileName) {
		size_t len = strLen(CPtr(fileName));
		if (len == 0)
			return 0;
		const char *fileNameStr = strData(CPtr(fileName));

		char c0 = fileNameStr[0];

		if (len == 1) {
			if (c0 == '~')
				return 2;  // return a length greater than the input
			return isSeparator(c0) ? 1 : 0;
		} else {
			if (c0 == '~') {
				ptrdiff_t posUnix = String::indexOf(fileName, UNIX_SEPARATOR, 1);
				ptrdiff_t posWin = String::indexOf(fileName, WINDOWS_SEPARATOR, 1);
				if (posUnix < 0 && posWin < 0)
					return (ptrdiff_t)len + 1;  // return a length greater than the input

				posUnix = posUnix < 0 ? posWin : posUnix;
				posWin = posWin < 0 ? posUnix : posWin;
				return std::min(posUnix, posWin) + 1;
			}

			char c1 = fileNameStr[1];
			if (c1 == ':') {
				c0 = (char)toupper(c0);
				if (c0 >= 'A' && c0 <= 'Z') {
					if (len == 2 || isSeparator(fileNameStr[2]) == false)
						return 2;
					return 3;
				} else if (c0 == UNIX_SEPARATOR)
					return 1;
				return -1;
			} else if (isSeparator(c0) && isSeparator(c1)) {
				ptrdiff_t posUnix = String::indexOf(fileName, UNIX_SEPARATOR, 2);
				ptrdiff_t posWin = String::indexOf(fileName, WINDOWS_SEPARATOR, 2);
				if ((posUnix == -1 && posWin == -1) || posUnix == 2 || posWin == 2)
					return -1;
				posUnix = posUnix < 0 ? posWin : posUnix;
				posWin = posWin < 0 ? posUnix : posWin;
				return std::min(posUnix, posWin) + 1;
			} else
				return isSeparator(c0) ? 1 : 0;
		}
	}

private:
	/**
	 * Performs normalization on the file name
	 * @param fileName  file name
	 * @param sep  separator character
	 * @param keepSep  set to \c true to keep the final separator
	 * @return the normalized file name
	 */
	template <class S>
	static UPtr<String> doNormalize(S const& fileName, char sep, bool keepSep) {
		const char *fileNameStr = strData(CPtr(fileName));
		if (!fileNameStr)
			return nullptr;
		ptrdiff_t fileNameLen = strLen(CPtr(fileName));
		if (fileNameLen == 0)
			return ""_UPTR;

		int prefixLen = getPrefixLength(fileName);
		if (prefixLen < 0)
			throw InvalidPathException(_HERE_, fmt::format("Invalid path: '{}'", StringView(fileNameStr, fileNameLen)).c_str());
		int prefix = prefixLen;

		std::vector<char> array(fileNameLen + 2);
		memcpy(array.data(), fileNameStr, fileNameLen * sizeof(char));

		// fix separators throughout
		char otherSep = (sep == SYSTEM_SEPARATOR) ? OTHER_SEPARATOR : SYSTEM_SEPARATOR;
		for (unsigned int i = 0; i < array.size(); i++) {
			if (array[i] == otherSep)
				array[i] = sep;
		}

		// add extra separator on the end to simplify code below
		bool lastIsDir = true;
		if (array[fileNameLen - 1] != sep) {
			array[fileNameLen++] = sep;
			lastIsDir = false;
		}

		// //
		for (int i = prefix + 1; i < fileNameLen; i++) {
			if (array[i] == sep && array[i - 1] == sep) {
				array.erase(array.begin() + (ptrdiff_t)i - 1);
				fileNameLen--;
				i--;
			}
		}

		// ./
		for (int i = prefix + 1; i < fileNameLen; i++) {
			if (array[i] == sep && array[i - 1] == '.' && (i == prefix + 1 || array[i - 2] == sep)) {
				if (i == fileNameLen - 1)
					lastIsDir = true;
				array.erase(array.begin() + (ptrdiff_t)i - 1, array.begin() + (ptrdiff_t)i + 1);
				fileNameLen -= 2;
				i--;
			}
		}

		// ../
		for (int i = prefix + 2; i < fileNameLen; i++) {
			if (array[i] == sep && array[i - 1] == '.' && array[i - 2] == '.' && (i == prefix + 2 || array[i - 3] == sep)) {
				if (i == prefix + 2)
					throw InvalidPathException(_HERE_, fmt::format("Invalid path after normalization: '{}'", StringView(fileNameStr, fileNameLen)).c_str());

				if (i == fileNameLen - 1)
					lastIsDir = true;

				int j;
				bool continueOuter = false;

				for (j = i - 4 ; j >= prefix; j--) {
					if (array[j] == sep) {
						// remove b/../ from a/b/../c
						array.erase(array.begin() + (ptrdiff_t)j + 1, array.begin() + (ptrdiff_t)i + 1);
						fileNameLen -= i - j;
						i = j + 1;
						continueOuter = true;
						break;
					}
				}

				if (continueOuter)
					continue;

				// remove a/../ from a/../c
				array.erase(array.begin() + (ptrdiff_t)prefix, array.begin() + (ptrdiff_t)i + 1);
				fileNameLen -= i + 1 - prefix;
				i = prefix + 1;
			}
		}

		if (fileNameLen <= 0)  // should never be less than 0
			return ""_UPTR;

		if (fileNameLen <= prefix) {  // should never be less than prefix
			return newU<String>(array.data(), fileNameLen);
		}

		if (lastIsDir && keepSep)
			return newU<String>(array.data(), fileNameLen); // keep trailing separator

		return newU<String>(array.data(), fileNameLen - 1); // drop trailing separator
	}

	template <class S>
	static ptrdiff_t indexOfLastSeparator(S const* fileName) {
		return String::lastIndexOf(fileName, UNIX_SEPARATOR);
	}

	template <class S>
	static ptrdiff_t indexOfExtSep(S const* fileName) {
		ptrdiff_t extPos = String::lastIndexOf(fileName, '.');
		ptrdiff_t lastSep = indexOfLastSeparator(fileName);
		if (lastSep > extPos)
			return -1;
		return extPos;
	}

public:
	/** @throws InvalidPathException */
	template <class S>
	static UPtr<String> normalize(S const& fileName) {
		return doNormalize(fileName, SYSTEM_SEPARATOR, true);
	}

	template <class S>
	static UPtr<String> getExtension(S const* fileName) {
		if (!fileName)
			return nullptr;
		ptrdiff_t index = indexOfExtension(fileName);
		if (index < 0)
			return ""_UPTR;
		return String::substring(fileName, (size_t)index + 1);
	}

	template <class S>
	static bool isPathAbsolute(S const& path) {
		return String::startsWith(path, UNIX_SEPARATOR);
	}

	/*template <class S1, class S2>
	static UPtr<String> buildPath(S1 const* dir, S2 const* name) {
		StringBuilder path(String::strRaw(name), String::strLen(name));
		if (StringUtils::isEmpty(dir))
			return newU<String>(name);
		if (!isPathAbsolute(name)) {
			size_t dirLen = String::strLen(dir);
			if (dirLen > 0 && String::endsWith(dir, SYSTEM_SEPARATOR))
				path.clear().addStr(dir).add(name);
			else
				path.clear().addStr(dir).add(SYSTEM_SEPARATOR).add(name);
		}
		return path.toString();
	}*/

	/**
	 * Concatenates a fileName to a base path using normal command line style rules.
	 * <p>
	 * The effect is equivalent to resultant directory after changing
	 * directory to the first argument, followed by changing directory to
	 * the second argument.
	 * <p>
	 * The first argument is the base path, the second is the path to concatenate.
	 * The returned path is always normalized via {@link #normalize(String)},
	 * so <code>..</code> is handled.
	 * <p>
	 * If <code>pathToAdd</code> is absolute (has an absolute prefix), then
	 * it will be normalized and returned.
	 * Otherwise, the paths will be joined, normalized and returned.
	 * <p>
	 * The output will be the same on both Unix and Windows except
	 * for the separator character.
	 * <pre>
	 * /foo/      + bar        --&gt;  /foo/bar
	 * /foo       + bar        --&gt;  /foo/bar
	 * /foo       + /bar       --&gt;  /bar
	 * /foo/a/    + ../bar     --&gt;  /foo/bar
	 * /foo/      + ../../bar  --&gt;  null
	 * /foo/      + /bar       --&gt;  /bar
	 * /foo/..    + /bar       --&gt;  /bar
	 * /foo       + bar/c.txt  --&gt;  /foo/bar/c.txt
	 * /foo/c.txt + bar        --&gt;  /foo/c.txt/bar (!)
	 * </pre>
	 * (!) Note that the first parameter must be a path. If it ends with a name, then
	 * the name will be built into the concatenated path.
	 *
	 * @param basePath  the base path to attach to, always treated as a path
	 * @param fullFileNameToAdd  the fileName (or path) to attach to the base
	 * @return the concatenated path, or \c nullptr if invalid.
	 */
	template <class S1, class S2>
	static UPtr<String> concat(S1 const& basePath, S2 const& fullFileNameToAdd) {
		int prefix = getPrefixLength(fullFileNameToAdd);
		if (prefix < 0)
			return nullptr;
		if (prefix > 0)
			return normalize(fullFileNameToAdd);
		if (!CPtr(basePath))
			return nullptr;

		size_t baseLen = strLen(CPtr(basePath));
		if (baseLen == 0)
			return normalize(fullFileNameToAdd);

		size_t fileNameLen = strLen(CPtr(fullFileNameToAdd));

		const char *basePathStr = strData(CPtr(basePath));
		char ch = basePathStr[baseLen - 1];
		if (isSeparator(ch)) {
			StringBuilder path(baseLen + fileNameLen);
			path.addStr(basePath).addStr(fullFileNameToAdd);
			return normalize(path);
		}

		StringBuilder path(baseLen + 1 + fileNameLen);
		path.addStr(basePath).add(UNIX_SEPARATOR).addStr(fullFileNameToAdd);

		return normalize(path);
	}

	template <class S>
	static UPtr<String> getPath(S const* fileName) {
		ptrdiff_t lastSep = String::lastIndexOf(fileName, '/');
		if (lastSep < 0)
			return newU<String>(fileName);
		return String::substring(fileName, 0, (size_t)lastSep);
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_FILENAMEUTILS_H
