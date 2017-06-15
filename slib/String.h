/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_STRING_H__
#define __SLIB_STRING_H__

#include "libstdinclude.h"
#include "slib/exception/Exception.h"
#include "slib/List.h"

#include "fmt/format.h"

#include <string>
#include <stddef.h>
#include <string.h>

namespace slib {

/** Thrown by String methods to indicate that an index is either negative or greater than the size of the string. */
class StringIndexOutOfBoundsException : public IndexOutOfBoundsException {
public:
	StringIndexOutOfBoundsException(const char *where, std::ptrdiff_t index)
	:IndexOutOfBoundsException(where, fmt::format("String index out of range: {:d}", index).c_str()) {}
};

class String {
protected:
	static ptrdiff_t lastIndexOf(const char *source, ptrdiff_t sourceOffset, size_t sourceCount,
								 const char *target, ptrdiff_t targetOffset, size_t targetCount,
								 ptrdiff_t fromIndex) {
		ptrdiff_t rightIndex = sourceCount - targetCount;

		if (fromIndex < 0)
			return -1;
		if (fromIndex > rightIndex)
			fromIndex = rightIndex;

		if (targetCount == 0)
			return fromIndex;

		ptrdiff_t strLastIndex = targetOffset + targetCount - 1;
		char strLastChar = target[strLastIndex];
		ptrdiff_t min = sourceOffset + targetCount - 1;
		ptrdiff_t i = min + fromIndex;

		while (true) {
			while (i >= min && source[i] != strLastChar)
				i--;
			if (i < min)
				return -1;
			ptrdiff_t j = i - 1;
			ptrdiff_t start = j - (targetCount - 1);
			ptrdiff_t k = strLastIndex - 1;

			bool continueOuter = false;
			while (j > start) {
				if (source[j--] != target[k--]) {
					i--;
					continueOuter = true;
					break;
				}
			}
			if (continueOuter)
				continue;
			return start - sourceOffset + 1;
		}
	}
public:
	static bool equals(const std::string& str, const std::string& other) {
		const char *buffer = str.c_str();
		size_t len = str.size();
		
		if (buffer == other.c_str())
			return true;
		if (len == other.size())
			return !strcmp(buffer, other.c_str());
		return false;
	}

	static bool startsWith(const std::string& str, char prefix) {
		const char *buffer = str.c_str();
		return buffer[0] == prefix;
	}

	static bool startsWith(const std::string& str, const std::string& prefix) {
		const char *buffer = str.c_str();
		return (strncmp(buffer, prefix.c_str(), prefix.size()) == 0);
	}

	static bool startsWithIgnoreCase(const std::string& str, const std::string& prefix) {
		const char *buffer = str.c_str();
		return (strnicmp(buffer, prefix.c_str(), prefix.size()) == 0);
	}

	static bool startsWith(const std::string& str, const std::string &prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;
		size_t len = str.size();
		if (uOffset > len - prefix.size())
			return false;
		const char *buffer = str.c_str();
		return (strncmp(buffer + uOffset, prefix.c_str(), prefix.size()) == 0);
	}

	static bool startsWithIgnoreCase(const std::string& str, const std::string &prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;
		size_t len = str.size();
		if (uOffset > len - prefix.size())
			return false;
		const char *buffer = str.c_str();
		return (strnicmp(buffer + uOffset, prefix.c_str(), prefix.size()) == 0);
	}

	static bool endsWith(const std::string& str, char suffix) {
		const char *buffer = str.c_str();
		size_t len = str.size();
		if ((buffer == nullptr) || (len == 0))
			return false;
		return buffer[len - 1] == suffix;
	}

	static bool endsWith(const std::string& str, const std::string &suffix) {
		size_t len = str.size();
		return (startsWith(suffix, len - suffix.size()));
	}

	static std::string trim(const char *buffer, size_t len) {
		size_t a = 0;
		ptrdiff_t b = len;
		for (a = 0; (a < len) && isspace(buffer[a]); a++);
		for (b = len; (b > 0) && isspace(buffer[b - 1]); b--);

		if (b > (ptrdiff_t)a)
			return substring(buffer, len, a, b);
		else
			return "";
	}

	static std::string trim(const std::string& str) {
		return trim(str.c_str(), str.length());
	}

	static std::string trim(const char *str) {
		return trim(str, strlen(str));
	}

	static ptrdiff_t indexOf(const std::string& str, char ch) {
		const char *buffer = str.c_str();

		const char *pos = strchr(buffer, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	static ptrdiff_t indexOf(const std::string& str, char ch, size_t fromIndex) {
		const char *buffer = str.c_str();
		size_t len = str.size();

		if (fromIndex >= len)
			return -1;

		const char *pos = strchr(buffer + fromIndex, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	static ptrdiff_t indexOf(const std::string& str, const std::string& sub) {
		const char *buffer = str.c_str();

		const char *pos = strstr(buffer, sub.c_str());
		return (pos == nullptr ? -1 : pos - buffer);
	}

	static ptrdiff_t indexOf(const std::string& str, const std::string& sub, size_t fromIndex) {
		const char *buffer = str.c_str();
		size_t len = str.size();

		if (fromIndex >= len)
			return -1;

		const char *pos = strstr(buffer + fromIndex, sub.c_str());
		return (pos == nullptr ? -1 : pos - buffer);
	}

	static ptrdiff_t lastIndexOf(const std::string& str, char ch) {
		size_t len = str.size();
		return lastIndexOf(str, ch, len - 1);
	}

	static ptrdiff_t lastIndexOf(const std::string& str, char ch, ptrdiff_t fromIndex) {
		if (fromIndex < 0)
			return -1;

		const char *buffer = str.c_str();
		size_t len = str.size();

		int min = 0;
		const char *v = buffer;

		ptrdiff_t i = (((size_t)fromIndex >= len) ? len - 1 : fromIndex);

		for (; i >= min ; i--) {
			if (v[i] == ch)
				return i;
		}
		return -1;
	}

	static ptrdiff_t lastIndexOf(const std::string& str, const std::string& sub) {
		const char *buffer = str.c_str();
		size_t len = str.size();
		return lastIndexOf(buffer, 0, len, sub.c_str(), 0, sub.size(), len);
	}

	static std::string substring(const char *buffer, size_t len, size_t beginIndex, size_t endIndex) {
		if (endIndex > len)
			throw StringIndexOutOfBoundsException(_HERE_, endIndex);
		if (beginIndex > endIndex)
			throw StringIndexOutOfBoundsException(_HERE_, endIndex - beginIndex);

		size_t outLen = endIndex - beginIndex;
		return std::string(buffer + beginIndex, outLen);
	}

	static std::string substring(const std::string& str, size_t beginIndex, size_t endIndex) {
		return substring(str.c_str(), str.length(), beginIndex, endIndex);
	}

	static std::string substring(const std::string& str, size_t beginIndex) {
		return substring(str, beginIndex, str.size());
	}

	/*static void simpleSplit(const std::string& str, List<std::string> &results, const char delim, int limit = 65535) {
		const char *buffer = str.c_str();
		size_t len = str.size();
		
		if (len == 0)
			return;

		int nRet = 0;

		const char *ptr = buffer;
		//while (*ptr != 0) {
		while(true) {
			const char *nextDelim = strchr(ptr, delim);
			if (nextDelim == nullptr) {
				results.add(ptr);
				nRet++;
				return;
			} else {
				if (nRet <= limit - 1) {
					results.add(std::string(ptr, nextDelim - ptr));
					nRet++;
					ptr = nextDelim + 1;
				} else {
					results.add(ptr);
					nRet++;
					return;
				}
			}
		}
	}*/

	static void simpleSplit(const std::string& str, List<std::string> &results, const char delim, int limit = 65535) {
		const char *buffer = str.c_str();
		size_t len = str.size();

		if (len == 0)
			return;

		const char *nextDelim;
		const char *ptr = buffer;
		do {
			if (--limit == 0) {
				results.add(ptr);
				return;
			}
			nextDelim = strchr(ptr, delim);
			if (nextDelim == nullptr)
				results.add(ptr);
			else
				results.add(std::string(ptr, nextDelim - ptr));
			ptr = nextDelim + 1;
		} while (nextDelim != nullptr);
	}
};

} // namespace

#endif
