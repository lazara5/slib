/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_STRING_H__
#define __SLIB_STRING_H__

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
		return (strncasecmp(buffer, prefix.c_str(), prefix.size()) == 0);
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
		return (strncasecmp(buffer + uOffset, prefix.c_str(), prefix.size()) == 0);
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

/**
 * Immutable ASCII string with case-insensitive comparison and hash code
 */
class ASCIICaseInsensitiveString {
private:
	static const unsigned char _toLower[];
protected:
	unsigned char *_buffer;
	size_t _len;
	mutable volatile int32_t _hash;
public:
	ASCIICaseInsensitiveString();
	ASCIICaseInsensitiveString(const char *str, std::ptrdiff_t len = -1);

	ASCIICaseInsensitiveString(std::tuple<const char *, std::ptrdiff_t> t)
	:ASCIICaseInsensitiveString(std::get<0>(t), std::get<1>(t)) {}

	ASCIICaseInsensitiveString(const char *str, size_t offset, std::ptrdiff_t count);

	ASCIICaseInsensitiveString(std::tuple<const char *, size_t, std::ptrdiff_t> t)
	:ASCIICaseInsensitiveString(std::get<0>(t), std::get<1>(t), std::get<2>(t)) {}

	ASCIICaseInsensitiveString(const ASCIICaseInsensitiveString &other);
	ASCIICaseInsensitiveString(const std::string& other);

	~ASCIICaseInsensitiveString();

	size_t length() const {
		return _len;
	}

	bool isEmpty() const {
		return (isNull() || _len == 0);
	}

	std::tuple<const char*, std::ptrdiff_t> tuple() const {
		return std::make_tuple(c_str(), length());
	}

	bool operator==(const ASCIICaseInsensitiveString& other) const;
	bool operator==(const String& other) const;

	ASCIICaseInsensitiveString& operator=(const ASCIICaseInsensitiveString&) = delete;

	// move assignment
	ASCIICaseInsensitiveString& operator=(ASCIICaseInsensitiveString &&other) = delete;

	// bool operator
	explicit operator bool() const {
		return !isNull();
	}

	// for Java compatibility

	/**
	 * Compares this String to the specified String object. Returns <i>true</i>
	 * only if the argument is a String object that contains the same sequence
	 * of characters as this String or if both this and the other String
	 * object are <i>'NULL'</i> references.
	 * @param other The String object to compare this String against
	 * @return <i>true</i> if the given object represents a String object
	 *		equivalent to this String, <i>false</i> otherwise.
	 */
	virtual bool equals(const ASCIICaseInsensitiveString& other) const;

	/**
	 * Compares this String to another String, ignoring case.
	 * Two strings are considered equal ignoring case if they are of the same length and
	 * corresponding characters in the two strings are equal ignoring case.
	 * Uses stricmp() internally.
	 *
	 * @param other The String to compare this String against
	 *
	 * @return  <i><b>true</b></i> if the argument represents an equivalent String ignoring case;
	 *		<i><b>false</b></i> otherwise.
	 *
	 * @see equals(const String&)
	 */
	bool equalsIgnoreCase(const ASCIICaseInsensitiveString& other) const;

	/**
	 * Returns a hash code for this string. The hash code for a String object is computed as
	 * <i>s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]</i>
	 * using signed int arithmetic, where <i>s[i]</i> is the <i>i</i>th character in the string,
	 * <i>n</i> is the length of the string and <i>^</i> indicates exponentiation.
	 * The hash value of an empty string is zero.
	 * @return  a hash code value for this object.
	 */
	virtual int hashCode() const;

	/**
	 * Creates a <i>'NULL'</i> reference to a String.
	 * @return a <i>'NULL'</i> reference to a String.
	 */
	static ASCIICaseInsensitiveString& getNull();

	/**
	 * Checks if this String objects represents a <i>'NULL'</i> reference.
	 * @return <i>true</i> if the String represents a <i>'NULL'</i> reference.
	 */
	bool isNull() const {
		return (_buffer == nullptr);
	}

	/**
	 * Get constant C string (null-terminated)
	 * @return pointer to constant C string
	 */
	const char *c_str() const {
		return (const char*)_buffer;
	}
};

extern ASCIICaseInsensitiveString NULLASCIICISTRING;

} // namespace

namespace std {
	// for using ASCIICaseInsensitiveString as key in unordered_map
	template<> struct hash<slib::ASCIICaseInsensitiveString> {
		std::size_t operator()(const slib::ASCIICaseInsensitiveString& s) const {
			return (size_t)s.hashCode();
		}
	};
}

#endif
