/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_STRINGBUILDER_H
#define H_SLIB_STRINGBUILDER_H

#include "slib/String.h"
#include "slib/exception/NullPointerException.h"

#include <string>
#include <sstream>
#include <unordered_map>

namespace slib {

/**
 * Mutable string container (roughly equivalent with a Java <b>%String</b> + <b>%StringBuilder</b>, i.e. <b>NOT</b>
 * thread-safe when used in a mutable fashion !!!)
 */
class StringBuilder {
private:
	void build(const char *format, ...);
protected:
	unsigned char *_buffer;
	size_t _len, _size;
	mutable volatile int32_t _hash;

	void grow(size_t newLen);
	void internalAppend(const char *format, va_list ap);

	static std::ptrdiff_t lastIndexOf(const char *source, std::ptrdiff_t sourceOffset, size_t sourceCount,
									  const char *target, std::ptrdiff_t targetOffset, size_t targetCount,
									  std::ptrdiff_t fromIndex);
public:
	/** for std container compatibility */
	typedef char value_type;

	StringBuilder();
	StringBuilder(const char *str, std::ptrdiff_t len = -1);

	StringBuilder(std::tuple<const char *, std::ptrdiff_t> t)
	:StringBuilder(std::get<0>(t), std::get<1>(t)) {}

	StringBuilder(const char *str, size_t offset, size_t count);

	StringBuilder(std::tuple<const char *, size_t, std::ptrdiff_t> t)
	:StringBuilder(std::get<0>(t), std::get<1>(t), std::get<2>(t)) {}

	StringBuilder(const StringBuilder &other);
	StringBuilder(const std::string& other);

	/** move constructor */
	StringBuilder(StringBuilder &&other);

	virtual ~StringBuilder();

	// conversion constructors
	StringBuilder(int32_t val);
	StringBuilder(uint32_t val);
	StringBuilder(int64_t val);
	StringBuilder(uint64_t val);
	StringBuilder(double val);

	void alloc(size_t newLen);
	void setLength(size_t newLen);

	size_t length() const {
		return _len;
	}

	size_t size() const {
		return _size;
	}

	bool isEmpty() const {
		return (isNull() || _len == 0);
	}

	std::tuple<const char*, std::ptrdiff_t> tuple() const {
		return std::make_tuple(c_str(), length());
	}

	bool operator==(const StringBuilder& other) const;

	StringBuilder& operator=(const StringBuilder& other);

	/** move assignment */
	StringBuilder& operator=(StringBuilder &&other);

	// assignment
	void assign(const char *src, size_t len);
	void assignInternal(const char *src, size_t len);
	void clear();
	void truncate(size_t len);

	char *releaseBufferOwnership();

	// appenders
	StringBuilder& add(const char *src, std::ptrdiff_t len = -1);
	StringBuilder& add(const StringBuilder &src);
	StringBuilder& add(const ASCIICaseInsensitiveString &src);
	StringBuilder& add(const std::string& src);

	StringBuilder& add(const std::shared_ptr<std::string>& src) {
		if (src)
			add(*src);
		return *this;
	}

	StringBuilder& addLine(const char *src, std::ptrdiff_t len = -1);
	StringBuilder& add(int i);
	StringBuilder& add(int64_t i);
	StringBuilder& add(double d);

	StringBuilder& add(char c) {
		_hash = 0;
		if (_len + 1 >= _size) 
			grow(_len + 2);
		_buffer[_len] = (unsigned char)c;
		_len++;
		_buffer[_len] = 0;
		return *this;
	}

	/** for std container compatibility */
	void push_back(char c) {
		add(c);
	}
	
	StringBuilder& addFmt(const char *format, ...)
		__attribute__((format (printf, 2, 3)));
	StringBuilder& addFmtVL(const char *format, va_list ap)
		__attribute__((format (printf, 2, 0)));
	StringBuilder& addFmtLine(const char *format, ...)
		__attribute__((format (printf, 2, 3)));

	// appender operators
	StringBuilder& operator +=(const StringBuilder& op);
	const StringBuilder operator+(const StringBuilder& other) const;

	StringBuilder& operator +=(const char* op);
	const StringBuilder operator+(const char* other) const;

	StringBuilder& operator +=(char op);
	const StringBuilder operator+(char other) const;

	StringBuilder& operator +=(int op);
	const StringBuilder operator+(int other) const;

	StringBuilder& operator +=(int64_t op);
	const StringBuilder operator+(int64_t other) const;

	StringBuilder& operator +=(double op);
	const StringBuilder operator+(double other) const;

	bool operator <(const StringBuilder& other) const;

	/** bool operator */
	explicit operator bool() const {
		return !isNull();
	}

	// stream versions
	StringBuilder& operator <<(const StringBuilder& src);
	StringBuilder& operator <<(char c);
	StringBuilder& operator <<(int i);
	StringBuilder& operator <<(int64_t i);
	StringBuilder& operator <<(double d);

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
	virtual bool equals(const StringBuilder& other) const;

	/**
	 * Compares this String to another String, ignoring case.
	 * Two strings are considered equal ignoring case if they are of the same length and 
	 * corresponding characters in the two strings are equal ignoring case. 
	 * Uses stricmp() internally.
	 *
	 * @param other The String to compare this String to
	 *
	 * @return  <i><b>true</b></i> if the argument represents an equivalent String ignoring case; 
	 *		<i><b>false</b></i> otherwise.
	 *
	 * @see equals(const String&)
	 */
	bool equalsIgnoreCase(const StringBuilder& other) const;

	/**
	 * Returns the character at the specified offset. The index ranges from <i>0</i> to
	 * <i>length() - 1</i>.
	 *
	 * @param index zero-based index in this String.
	 * @return the char value at the specified index of this String.
	 * @throws  N/A <b>Will crash</b> if the <i>index</i>
	 *		argument is negative or not less than the length of this string.
	 */
	char charAt(size_t pos) const;

	/**
	 * Returns the offset in this String of the first occurrence of the specified character. 
	 *
	 * @param ch a character
	 * @return the index of the first occurrence of the character in the
	 *		character sequence represented by this String or
	 *      <i><b>-1</b></i> if the character does not occur.
	 */
	std::ptrdiff_t indexOf(char ch) const;

	/**
	 * Returns the index within this StringBuilder of the first occurrence of the specified character,
	 * starting at the specified index.
	 * If <i>fromIndex</i> is greater than the length of this StringBuilder, <i><b>-1</b></i> is returned.
	 *
	 * @param ch  a character.
	 * @param fromIndex  the index to start the search from.
	 * @return the index of the first occurrence of the character in the
	 *		character sequence represented by this String that is >= to 
	 *		<i>fromIndex</i> or <i><b>-1</b></i> if the character does not occur.
	 */
	std::ptrdiff_t indexOf(char ch, size_t fromIndex) const;

	/**
	 * Returns the offset in this String of the first occurrence of the specified substring.
	 *
	 * @param sub  a string.
	 * @return if the string argument appears as a substring within this StringBuilder, then the
	 *		index of the first character of the first occurence is returned; if it does not,
	 *      then <i><b>-1</b></i> is returned.
	 */
	std::ptrdiff_t indexOf(const StringBuilder& sub) const;

	/**
	 * Returns the offset in this String of the first occurrence of the
	 * specified substring, starting at the specified index. 
	 *
	 * @param sub a string.
	 * @return if the string argument appears as a substring within this String, starting at 
	 *		the specified offset, then the index of the first character of the first occurence 
	 *		is returned; if it does not, then <i><b>-1</b></i> is returned.
	 */
	std::ptrdiff_t indexOf(const StringBuilder& sub, size_t fromIndex) const;

	/**
	 * Returns the index within this string of the last occurrence of
	 * the specified character. The String is searched backwards 
	 * starting with the last character.
	 * @param ch  a character.
	 * @return the index of the last occurrence of the character in the
	 *		character sequence represented by this String, or
	 *		<i><b>-1</b></i> if the character does not occur.
	 */
	std::ptrdiff_t lastIndexOf(char ch) const;

	/**
	 * Returns the index within this string of the last occurrence of
	 * the specified character, searching backward starting from the
	 * specified index.
	 *
	 * @param ch  a character 
	 * @param fromIndex  the index to start the search from. If <i>fromIndex</i> 
	 *			is greater than or equal to the length of this String, this entire 
	 *			String may be searched. If it is negative, <i><b>-1</b></i> is returned.
	 * @return the index of the last occurrence of the character in the
	 *			character sequence represented by this object that is less
	 *			than or equal to <i>fromIndex</i>, or <i><b>-1</b></i>
	 *			if the character does not occur before that point.
	 */
	std::ptrdiff_t lastIndexOf(char ch, std::ptrdiff_t fromIndex) const;

	/**
	 * Returns the index within this string of the rightmost occurrence of the 
	 * specified substring. The rightmost empty string "" is considered to occur at the 
	 * index value <i>this->length()</i>.
	 * The returned index is the largest value <i>k</i> such that:
	 * this->startsWith(str, <i>k</i>) is <i>true</i>.
	 * @param sub  the substring to search for.
	 * @return if the string argument occurs one or more times as a substring
	 *		within this object, then the index of the first character of
	 *      the last such substring is returned. If it does not occur as
	 *      a substring, <i><b>-1</b></i> is returned.
	 */
	std::ptrdiff_t lastIndexOf(const StringBuilder &sub) const;

	/**
     * Tests if this string starts with the specified prefix.
     * @param prefix the prefix.
     * @return  <i><b>true</b></i> if the character represented by the
     *		argument is a prefix of the character sequence represented by
     *      this string; <i><b>false</b></i> otherwise.
     */
	bool startsWith(char prefix) const;

	/**
     * Tests if this string starts with the specified prefix.
     * @param prefix the prefix.
     * @return  <i><b>true</b></i> if the character sequence represented by the
     *		argument is a prefix of the character sequence represented by
     *      this string; <i><b>false</b></i> otherwise.
     *      Note also that <i><b>true</b></i> will be returned if the argument is an empty string.
     */
	bool startsWith(const StringBuilder &prefix) const;

	/**
     * Tests if the substring of this string beginning at the
     * specified index starts with the specified prefix.
     * @param prefix  the prefix.
     * @param offset  where to begin looking in this string.
     * @return  <i><b>true</b></i> if the character sequence represented by the
     *		argument is a prefix of the substring of this object starting
     *      at index <i>offset</i>; <i><b>false</b></i> otherwise.
     *      The result is <i><b>false</b></i> if <i>offset</i> is
     *      negative or greater than the length of this String object; otherwise 
	 *		the result is the same as the result of the expression
     *      <i>this->substring(offset).startsWith(prefix)</i>
     */
	bool startsWith(const StringBuilder &prefix, std::ptrdiff_t offset) const;

	/**
	 * Tests if this string ends with the specified sufffix.
	 * @param suffix  the suffix.
	 * @return  <i><b>true</b></i> if the character represented by the
	 *		argument is a suffix of the character sequence represented by
	 *      this string; <i><b>false</b></i> otherwise.
	 */
	bool endsWith(char suffix) const;

	/**
	 * Tests if this string ends with the specified suffix.
	 * @param suffix  the suffix.
	 * @return <i><b>true</b></i> if the character sequence represented by the
	 *		argument is a suffix of the character sequence represented by
	 *      this object; <i><b>false</b></i> otherwise. Note that the
	 *      result will be <i><b>true</b></i> if the argument is the
	 *      empty string or is equal to this String object.
	*/
	bool endsWith(const StringBuilder &suffix) const;

	/**
	 * Returns a new String that is a substring of this String. The
	 * substring begins with the character at the specified index and
	 * extends to the end of this string. 
	 *
	 * @param beginIndex  the beginning index, inclusive.
	 * @return the specified substring.
	 * @throws IndexOutOfBoundsException  if
	 *             <code>beginIndex</code> is negative or larger than the
	 *             length of this <code>String</code> object.
	 */
	StringBuilder substring(size_t beginIndex) const;

	/**
     * Returns a new string that is a substring of this string. The
     * substring begins at the specified <code>beginIndex</code> and
     * extends to the character at index <code>endIndex - 1</code>.
     * Thus the length of the substring is <code>endIndex-beginIndex</code>.
     *
     * @param beginIndex  the beginning index, inclusive.
     * @param endIndex  the ending index, exclusive.
     * @return the specified substring.
     * @throws IndexOutOfBoundsException  if the <code>beginIndex</code> is negative, or
     *             <code>endIndex</code> is larger than the length of
     *             this <code>String</code> object, or <code>beginIndex</code> is larger than
     *             <code>endIndex</code>.
     */
	StringBuilder substring(size_t beginIndex, size_t endIndex) const;

	/**
	 * Trims whitespace characters from the ends of this string.
	 * @return a copy of the string, with leading and trailing whitespace omitted.
	 */
	StringBuilder trim() const;
	StringBuilder toLowerCase() const;
	StringBuilder toUpperCase() const;

	/**
	 * Returns a hash code for this string. The hash code for a StringBuilder object is
	 * computed as <i>s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]</i>
	 * using signed int arithmetic, where <i>s[i]</i> is the <i>i</i>th character in the string,
	 * and <i>n</i> is the length of the string.
	 * The hash value of an empty string is zero.
	 * @return a hash code value for this object.
	 */
	virtual int hashCode() const;

	/** 
	 * Creates a <i>'NULL'</i> reference to a StringBuilder.
	 * @return a <i>'NULL'</i> reference to a StringBuilder.
	 */
	static StringBuilder &getNull();

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

	/**
	 * Get C string (null-terminated)
	 * @return pointer to C string
	 */
	char *str() { 
		return (char*)_buffer; 
	}

	// for compatibility with RapidJSON streams
	typedef char Ch;

	void Put(char c) {
		add(c);
	}

	void Flush() {}
};

class NullStringBuilderException : public Exception {
public:
	NullStringBuilderException()
	: Exception(_HERE_, "The NULL StringBuilder instance is not modifiable!") {}
};

class NullStringBuilder : public StringBuilder {
public:
	NullStringBuilder()
	:StringBuilder(nullptr) {}

	[[ noreturn ]] void alloc(size_t) { throw NullStringBuilderException(); }
	[[ noreturn ]] void setLength(size_t) { throw NullStringBuilderException(); }
	StringBuilder& operator=(const StringBuilder&) { throw NullStringBuilderException(); }
	StringBuilder& operator=(StringBuilder &&) { throw NullStringBuilderException(); }
	[[ noreturn ]] void assign(const char *, size_t) { throw NullStringBuilderException(); }
	[[ noreturn ]] void assignInternal(const char *, size_t) { throw NullStringBuilderException(); }
	[[ noreturn ]] void clear() { throw NullStringBuilderException(); }
	[[ noreturn ]] void truncate(size_t) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(const char, std::ptrdiff_t = -1) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(const StringBuilder&) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& addLine(const char *, std::ptrdiff_t = -1) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(char) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(int) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(int64_t) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& add(double) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& addFmt(const char *, ...) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& addFmtVL(const char *, va_list) { throw NullStringBuilderException(); }
	[[ noreturn ]] StringBuilder& addFmtLine(const char *, ...) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(const StringBuilder&) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(const char*) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(char) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(int) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(int64_t) { throw NullStringBuilderException(); }
	StringBuilder& operator+=(double) { throw NullStringBuilderException(); }
	StringBuilder& operator<<(const StringBuilder&) { throw NullStringBuilderException(); }
	StringBuilder& operator<<(char) { throw NullStringBuilderException(); }
	StringBuilder& operator<<(int) { throw NullStringBuilderException(); }
	StringBuilder& operator<<(int64_t) { throw NullStringBuilderException(); }
	StringBuilder& operator<<(double) { throw NullStringBuilderException(); }
	char *str() { throw NullStringBuilderException(); }
};

extern NullStringBuilder NULLSTRINGBUILDER;

} // namespace

namespace std {
	// for using StringBuilder as key in unordered_map
	template<> struct hash<slib::StringBuilder> {
		std::size_t operator()(const slib::StringBuilder& s) const {
			return (size_t)s.hashCode();
		}
	};
}

#endif // H_SLIB_STRINGBUILDER_H
