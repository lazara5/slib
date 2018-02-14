/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_STRING_H
#define H_SLIB_STRING_H

#include "slib/exception/Exception.h"
#include "slib/collections/List.h"

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
		ptrdiff_t rightIndex = (ptrdiff_t)(sourceCount - targetCount);

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
	template <class S1, class S2>
	static bool equals(S1 const& str, S2 const& other) {
		const char *buffer = str.c_str();
		size_t len = str.length();
		const char *otherBuffer = other.c_str();
		size_t otherLen = other.length();
		
		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return buffer == nullptr;
		if (buffer == otherBuffer)
			return true;
		if (len == otherLen)
			return !strcmp(buffer, otherBuffer);
		return false;
	}

	template <class S1, class S2>
	static bool equalsIgnoreCase(S1 const& str, S2 const& other) {
		const char *buffer = str.c_str();
		size_t len = str.length();
		const char *otherBuffer = other.c_str();
		size_t otherLen = other.length();

		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return (buffer == nullptr);
		if (buffer == otherBuffer)
			return true;
		if (len == otherLen)
			return !strcasecmp(buffer, otherBuffer);
		return false;
	}

	template <class S>
	static bool startsWith(S const& str, char prefix) {
		const char *buffer = str.c_str();

		if (buffer == nullptr)
			return false;
		return buffer[0] == prefix;
	}

	template <class S1, class S2>
	static bool startsWith(S1 const& str,  S2 const& prefix) {
		const char *buffer = str.c_str();

		if (!buffer)
			return false;
		return (strncmp(buffer, prefix.c_str(), prefix.length()) == 0);
	}

	template <class S1, class S2>
	static bool startsWithIgnoreCase(S1 const& str, S2 const& prefix) {
		const char *buffer = str.c_str();
		if (!buffer)
			return false;
		return (strncasecmp(buffer, prefix.c_str(), prefix.length()) == 0);
	}

	template <class S1, class S2>
	static bool startsWith(S1 const& str, S2 const& prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;

		size_t len = str.size();

		if (uOffset > len - prefix.length())
			return false;

		const char *buffer = str.c_str();

		return (strncmp(buffer + uOffset, prefix.c_str(), prefix.length()) == 0);
	}

	template <class S1, class S2>
	static bool startsWithIgnoreCase(S1 const& str, S2 const& prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;
		size_t len = str.length();
		if (uOffset > len - prefix.size())
			return false;
		const char *buffer = str.c_str();
		return (strncasecmp(buffer + uOffset, prefix.c_str(), prefix.length()) == 0);
	}

	template <class S>
	static bool endsWith(S const& str, char suffix) {
		const char *buffer = str.c_str();
		size_t len = str.length();

		if ((buffer == nullptr) || (len == 0))
			return false;
		return buffer[len - 1] == suffix;
	}

	template <class S1, class S2>
	static bool endsWith(S1 const& str, S2 const& suffix) {
		size_t len = str.length();

		return (startsWith(str, suffix, (ptrdiff_t)(len - suffix.length())));
	}

	static std::string trim(const char *buffer, size_t len) {
		if (!buffer)
			return "";
		size_t a = 0;
		ptrdiff_t b = (ptrdiff_t)len;
		for (a = 0; (a < len) && isspace(buffer[a]); a++);
		for (b = (ptrdiff_t)len; (b > 0) && isspace(buffer[b - 1]); b--);

		if (b > (ptrdiff_t)a)
			return substring(buffer, len, a, (size_t)b);
		else
			return "";
	}

	template <class S>
	static std::string trim(S const& str) {
		return trim(str.c_str(), str.length());
	}

	static std::string trim(const char *str) {
		return trim(str, strlen(str));
	}

	template <class S>
	static ptrdiff_t indexOf(S const& str, char ch) {
		const char *buffer = str.c_str();
		if (!buffer)
			return -1;

		const char *pos = strchr(buffer, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <class S>
	static ptrdiff_t indexOf(S const& str, char ch, size_t fromIndex) {
		const char *buffer = str.c_str();

		if (buffer == nullptr)
			return -1;

		size_t len = str.length();

		if (fromIndex >= len)
			return -1;

		const char *pos = strchr(buffer + fromIndex, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <class S1, class S2>
	static ptrdiff_t indexOf(S1 const& str,  S2 const& sub) {
		const char *buffer = str.c_str();

		if (buffer == nullptr)
			return -1;

		const char *pos = strstr(buffer, sub.c_str());
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <class S1, class S2>
	static ptrdiff_t indexOf(S1 const& str, S2 const& sub, size_t fromIndex) {
		const char *buffer = str.c_str();

		if (!buffer)
			return -1;

		size_t len = str.length();

		if (fromIndex >= len)
			return -1;

		const char *pos = strstr(buffer + fromIndex, sub.c_str());
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <class S>
	static ptrdiff_t lastIndexOf(S const& str, char ch) {
		size_t len = str.length();

		return lastIndexOf(str, ch, (ptrdiff_t)(len - 1));
	}

	template <class S>
	static ptrdiff_t lastIndexOf(S const& str, char ch, ptrdiff_t fromIndex) {
		if (fromIndex < 0)
			return -1;

		const char *buffer = str.c_str();

		if (buffer == nullptr)
			return -1;

		size_t len = str.length();

		int min = 0;
		const char *v = buffer;

		ptrdiff_t i = (((size_t)fromIndex >= len) ? (ptrdiff_t)(len - 1) : fromIndex);

		for (; i >= min ; i--) {
			if (v[i] == ch)
				return i;
		}
		return -1;
	}

	template <class S1, class S2>
	static ptrdiff_t lastIndexOf(S1 const& str, S2 const& sub) {
		const char *buffer = str.c_str();
		const char *subBuffer = sub.c_str();

		if ((buffer == nullptr) || (subBuffer == nullptr))
			return -1;

		size_t len = str.length();
		return String::lastIndexOf(buffer, 0, len, subBuffer, 0, sub.length(), (ptrdiff_t)len);
	}

	static std::string substring(const char *buffer, size_t len, size_t beginIndex, size_t endIndex) {
		if (endIndex > len)
			throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)endIndex);
		if (beginIndex > endIndex)
			throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)(endIndex - beginIndex));

		size_t outLen = endIndex - beginIndex;
		return std::string(buffer + beginIndex, outLen);
	}

	template <class S>
	static std::string substring(S const& str, size_t beginIndex, size_t endIndex) {
		return substring(str.c_str(), str.length(), beginIndex, endIndex);
	}

	template <class S>
	static std::string substring(S const& str, size_t beginIndex) {
		return substring(str, beginIndex, str.length());
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

	/*static void simpleSplit(const char *buffer, size_t len, List<std::string> &results, const char delim, int limit = 65535) {
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
	}*/

	static void simpleSplit(const char *buffer, size_t len, List<std::string> &results, const char delim, int limit = 65535) {
		if (len == 0)
			return;

		const char *nextDelim;
		const char *ptr = buffer;
		do {
			if (--limit == 0) {
				results.add(std::make_shared<std::string>(ptr, len));
				return;
			}
			nextDelim = (const char *)memchr(ptr, delim, len);
			if (nextDelim)
				len -= (size_t)(nextDelim - ptr + 1);
			if (nextDelim == nullptr) {
				results.add(std::make_shared<std::string>(ptr, len));
			} else {
				results.add(std::make_shared<std::string>(ptr, (size_t)(nextDelim - ptr)));
			}
			ptr = nextDelim + 1;
		} while (nextDelim != nullptr);
	}


	template <class S>
	static void simpleSplit(S const& str, List<std::string> &results, const char delim, int limit = 65535) {
		simpleSplit(str.c_str(), str.length(), results, delim, limit);
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

	ASCIICaseInsensitiveString(const char *str, size_t offset, size_t count);

	ASCIICaseInsensitiveString(std::tuple<const char *, size_t, std::ptrdiff_t> t)
	:ASCIICaseInsensitiveString(std::get<0>(t), std::get<1>(t), std::get<2>(t)) {}

	ASCIICaseInsensitiveString(const ASCIICaseInsensitiveString &other);
	ASCIICaseInsensitiveString(const std::string& other);

	virtual ~ASCIICaseInsensitiveString();

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

#endif // H_SLIB_STRING_H
