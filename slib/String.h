/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_STRING_H
#define H_SLIB_STRING_H

#include "slib/Object.h"
#include "slib/exception/Exception.h"
#include "slib/util/TemplateUtils.h"

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

class BasicString: virtual public Object {
public:
	static Class const* _class;
public:
	virtual size_t length() const = 0;
	virtual const char *c_str() const = 0;
};

template <class E>
class ArrayList;

class String : public BasicString {
public:
	static Class const* _class;
protected:
	std::string _str;
	mutable volatile int32_t _hash;
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

		ptrdiff_t strLastIndex = targetOffset + (ptrdiff_t)targetCount - 1;
		char strLastChar = target[strLastIndex];
		ptrdiff_t min = sourceOffset + (ptrdiff_t)targetCount - 1;
		ptrdiff_t i = min + fromIndex;

		while (true) {
			while (i >= min && source[i] != strLastChar)
				i--;
			if (i < min)
				return -1;
			ptrdiff_t j = i - 1;
			ptrdiff_t start = j - ((ptrdiff_t)targetCount - 1);
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
	String(std::string const& str);

	String(const char *buffer, size_t len);

	String(String const& other);

	virtual ~String() override;

	virtual Class const* getClass() const override {
		return STRINGCLASS();
	}

	size_t length() const override {
		return _str.length();
	}

	bool isEmpty() const {
		return _str.empty();
	}

	const char *c_str() const override {
		return _str.c_str();
	}
public:
	template <typename S1, typename S2>
	static bool equals(S1 str, S2 other) {
		const S1* pStr = constPtr(str);
		const S2* pOther = constPtr(other);

		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *otherBuffer = pOther? pOther->c_str() : nullptr;

		
		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return buffer == nullptr;
		if (buffer == otherBuffer)
			return true;

		size_t len = pStr->length();
		size_t otherLen = pOther->length();
		if (len == otherLen)
			return !strcmp(buffer, otherBuffer);
		return false;
	}

	template <typename S>
	bool equals(S other) const {
		return equals(_str, other);
	}

	bool operator==(String const& other) const {
		return equals(other);
	}

	template <typename S1, typename S2>
	static bool equalsIgnoreCase(S1 str, S2 other) {
		const S1* pStr = constPtr(str);
		const S2* pOther = constPtr(other);

		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *otherBuffer = pOther? pOther->c_str() : nullptr;

		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return (buffer == nullptr);
		if (buffer == otherBuffer)
			return true;

		size_t len = pStr->length();
		size_t otherLen = pOther->length();
		if (len == otherLen)
			return !strcasecmp(buffer, otherBuffer);
		return false;
	}

	template <typename S>
	bool equalsIgnoreCase(S other) {
		return equalsIgnoreCase(_str, other);
	}

	template <typename S>
	static bool startsWith(S str, char prefix) {
		const S* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;

		if (buffer == nullptr)
			return false;
		return buffer[0] == prefix;
	}

	bool startsWith(char prefix) {
		return startsWith(_str, prefix);
	}

	template <typename S1, typename S2>
	static bool startsWith(S1 str,  S2 prefix) {
		const S1* pStr = constPtr(str);
		const S2* pPrefix = constPtr(prefix);

		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *prefixBuffer = pPrefix? pPrefix->c_str() : nullptr;

		if ((!buffer) || (!prefixBuffer))
			return false;
		return (strncmp(buffer, prefixBuffer, pPrefix->length()) == 0);
	}

	template <typename S>
	bool startsWith(S prefix) {
		return startsWith(_str, prefix);
	}

	template <typename S1, typename S2>
	static bool startsWithIgnoreCase(S1 str, S2 prefix) {
		const S1* pStr = constPtr(str);
		const S2* pPrefix = constPtr(prefix);

		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *prefixBuffer = pPrefix? pPrefix->c_str() : nullptr;

		if ((!buffer) || (!prefixBuffer))
			return false;
		return (strncasecmp(buffer, prefixBuffer, pPrefix->length()) == 0);
	}

	template <typename S>
	bool startsWithIgnoreCase(S prefix) {
		return startsWithIgnoreCase(_str, prefix);
	}

	template <typename S1, typename S2>
	static bool startsWith(S1 str, S2 prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;

		const S1* pStr = constPtr(str);
		const S2* pPrefix = constPtr(prefix);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *prefixBuffer = pPrefix? pPrefix->c_str() : nullptr;

		if ((!buffer) || (!prefixBuffer))
			return false;

		size_t len = pStr->length();

		if (uOffset > len - pPrefix->length())
			return false;

		return (strncmp(buffer + uOffset, prefixBuffer, pPrefix->length()) == 0);
	}

	template <typename S>
	bool startsWith(S prefix, ptrdiff_t offset) {
		return startsWith(_str, prefix, offset);
	}

	template <typename S1, typename S2>
	static bool startsWithIgnoreCase(S1 str, S2 prefix, ptrdiff_t offset) {
		if (offset < 0)
			return false;
		size_t uOffset = (size_t) offset;

		const S1* pStr = constPtr(str);
		const S2* pPrefix = constPtr(prefix);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const char *prefixBuffer = pPrefix? pPrefix->c_str() : nullptr;

		if ((!buffer) || (!prefixBuffer))
			return false;

		size_t len = pStr->length();
		if (uOffset > len - pPrefix->length())
			return false;

		return (strncasecmp(buffer + uOffset, prefixBuffer, pPrefix->length()) == 0);
	}

	template <typename S>
	bool startsWithIgnoreCase(S prefix, ptrdiff_t offset) {
		return startsWithIgnoreCase(_str, prefix, offset);
	}

	template <typename S>
	static bool endsWith(S str, char suffix) {
		const S* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		size_t len = pStr ? pStr->length() : 0;

		if ((buffer == nullptr) || (len == 0))
			return false;
		return buffer[len - 1] == suffix;
	}

	bool endsWith(char suffix) {
		return endsWith(_str, suffix);
	}

	template <typename S1, typename S2>
	static bool endsWith(S1 str, S2 suffix) {
		const S1* pStr = constPtr(str);
		size_t len = pStr ? pStr->length() : 0;
		const S2* pSuffix = constPtr(suffix);

		if (!pSuffix)
			return false;

		return (startsWith(str, suffix, (ptrdiff_t)(len - pSuffix->length())));
	}

	template <typename S>
	bool endsWith(S suffix) {
		return endsWith(_str, suffix);
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

	template <typename S>
	static std::string trim(S str) {
		const S* pStr = constPtr(str);
		if (!pStr)
			return "";
		return trim(pStr->c_str(), pStr->length());
	}

	String trim() {
		return trim(_str);
	}

	static std::string trim(const char *str) {
		if (!str)
			return "";
		return trim(str, strlen(str));
	}

	template <typename S>
	static ptrdiff_t indexOf(S str, char ch) {
		const S* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		if (!buffer)
			return -1;

		const char *pos = strchr(buffer, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	ptrdiff_t indexOf(char ch) {
		return indexOf(_str, ch);
	}

	template <typename S>
	static ptrdiff_t indexOf(S str, char ch, size_t fromIndex) {
		const S* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;

		if (buffer == nullptr)
			return -1;

		size_t len = pStr->length();

		if (fromIndex >= len)
			return -1;

		const char *pos = strchr(buffer + fromIndex, ch);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	ptrdiff_t indexOf(char ch, size_t fromIndex) {
		return indexOf(_str, ch, fromIndex);
	}

	template <typename S1, typename S2>
	static ptrdiff_t indexOf(S1 str, S2 sub) {
		const S1* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const S2* pSub = constPtr(sub);
		const char *subBuffer = pSub ? pSub->c_str() : nullptr;

		if ((buffer == nullptr) || (subBuffer == nullptr))
			return -1;

		const char *pos = strstr(buffer, subBuffer);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <typename S>
	ptrdiff_t indexOf(S sub) {
		return indexOf(_str, sub);
	}

	template <typename S1, typename S2>
	static ptrdiff_t indexOf(S1 str, S2 sub, size_t fromIndex) {
		const S1* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const S2* pSub = constPtr(sub);
		const char *subBuffer = pSub ? pSub->c_str() : nullptr;

		if ((buffer == nullptr) || (subBuffer == nullptr))
			return -1;

		size_t len = pStr->length();

		if (fromIndex >= len)
			return -1;

		const char *pos = strstr(buffer + fromIndex, subBuffer);
		return (pos == nullptr ? -1 : pos - buffer);
	}

	template <typename S>
	ptrdiff_t indexOf(S sub, size_t fromIndex) {
		return indexOf(_str, sub, fromIndex);
	}

	template <typename S>
	static ptrdiff_t lastIndexOf(S str, char ch) {
		const S* pStr = constPtr(str);
		if (!pStr)
			return -1;
		size_t len = pStr->length();

		return lastIndexOf(str, ch, (ptrdiff_t)(len - 1));
	}

	ptrdiff_t lastIndexOf(char ch) {
		return lastIndexOf(_str, ch);
	}

	template <typename S>
	static ptrdiff_t lastIndexOf(S str, char ch, ptrdiff_t fromIndex) {
		if (fromIndex < 0)
			return -1;

		const S* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;

		if (buffer == nullptr)
			return -1;

		size_t len = pStr->length();

		int min = 0;
		const char *v = buffer;

		ptrdiff_t i = (((size_t)fromIndex >= len) ? (ptrdiff_t)(len - 1) : fromIndex);

		for (; i >= min ; i--) {
			if (v[i] == ch)
				return i;
		}
		return -1;
	}


	ptrdiff_t lastIndexOf(char ch, ptrdiff_t fromIndex) {
		return lastIndexOf(_str, ch, fromIndex);
	}

	template <typename S1, typename S2>
	static ptrdiff_t lastIndexOf(S1 str, S2 sub) {
		const S1* pStr = constPtr(str);
		const char *buffer = pStr ? pStr->c_str() : nullptr;
		const S2* pSub = constPtr(sub);
		const char *subBuffer = pSub ? pSub->c_str() : nullptr;

		if ((buffer == nullptr) || (subBuffer == nullptr))
			return -1;

		size_t len = pStr->length();
		return lastIndexOf(buffer, 0, len, subBuffer, 0, pSub->length(), (ptrdiff_t)len);
	}

	template <typename S>
	ptrdiff_t lastIndexOf(S sub) {
		return lastIndexOf(_str, sub);
	}

	static std::string substring(const char *buffer, size_t len, size_t beginIndex, size_t endIndex) {
		if (endIndex > len)
			throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)endIndex);
		if (beginIndex > endIndex)
			throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)(endIndex - beginIndex));

		size_t outLen = endIndex - beginIndex;
		return std::string(buffer + beginIndex, outLen);
	}

	template <typename S>
	static std::string substring(S str, size_t beginIndex, size_t endIndex) {
		const S* pStr = constPtr(str);
		if (!pStr)
			throw NullPointerException(_HERE_);
		return substring(pStr->c_str(), pStr->length(), beginIndex, endIndex);
	}

	String substring(size_t beginIndex, size_t endIndex) {
		return substring(_str, beginIndex, endIndex);
	}

	template <typename S>
	static std::string substring(S str, size_t beginIndex) {
		const S* pStr = constPtr(str);
		if (!pStr)
			throw NullPointerException(_HERE_);
		return substring(str, beginIndex, pStr->length());
	}

	String substring(size_t beginIndex) {
		return substring(_str, beginIndex);
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



	static std::unique_ptr<ArrayList<std::string>> simpleSplit(const char *buffer, size_t len, const char delim, int limit = 65535);

	template <typename S>
	static std::unique_ptr<ArrayList<std::string>> simpleSplit(S str, const char delim, int limit = 65535) {
		const S* pStr = constPtr(str);
		if (!pStr)
			throw NullPointerException(_HERE_);
		return simpleSplit(pStr->c_str(), pStr->length(), delim, limit);
	}

	static std::unique_ptr<ArrayList<String>> split(const char *buffer, size_t len, const char *pattern, int limit = 0);

	std::unique_ptr<ArrayList<String>> split(const char *pattern, int limit = 65535);
};

/**
 * Immutable ASCII string with case-insensitive comparison and hash code
 */
class ASCIICaseInsensitiveString : public BasicString {
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

	virtual ~ASCIICaseInsensitiveString() override;

	size_t length() const override {
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
	virtual int32_t hashCode() const override;

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
	const char *c_str() const override {
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
