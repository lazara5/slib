/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/StringBuilder.h"
#include "slib/String.h"

#include <sstream>
#include <string>
#include <cmath>
#include <vector>

using std::ptrdiff_t;

namespace slib {

/** Extra initial allocation for internal buffer */
static const int _STR_EXTRA_ALLOC = 0;

/** Growth factor for reallocation */
//static const float _STR_GROWTH_FACTOR = 2;
static const float _STR_GROWTH_FACTOR = 1.5;

StringBuilder::StringBuilder() {
	_buffer = (unsigned char*)malloc(16);
	if (!_buffer)
		throw OutOfMemoryError(_HERE_);
	_buffer[0] = 0;
	_size = 16;
	_len = 0;
}

StringBuilder::StringBuilder(const char *str, ptrdiff_t len /* = -1 */) {
	if (str == nullptr) {
		if (len == -1) {
			_buffer = nullptr;
			_len = 0;
			_size = 0;
		} else {
			_buffer = (unsigned char*)malloc(len + 1);
			if (!_buffer)
				throw OutOfMemoryError(_HERE_);
			_buffer[0] = 0;
			_size = len + 1;
			_len = 0;
		}
	} else {
		if (len < 0)
			len = strlen(str);
		_buffer = (unsigned char*)malloc(len + _STR_EXTRA_ALLOC + 1);
		if (!_buffer)
			throw OutOfMemoryError(_HERE_);
		memcpy(_buffer, str, len);
		_len = len;
		_buffer[_len] = 0;
		_size = len + _STR_EXTRA_ALLOC + 1;
	}
}

StringBuilder::StringBuilder(const char *str, size_t offset, ptrdiff_t count) {
	_buffer = (unsigned char*) malloc(count + _STR_EXTRA_ALLOC + 1);
	if (!_buffer)
		throw OutOfMemoryError(_HERE_);
	if (count > 0)
		memcpy(_buffer, str + offset, count);
	_len = count;
	_buffer[_len] = 0;
	_size = count + _STR_EXTRA_ALLOC + 1;
}

StringBuilder::StringBuilder(const StringBuilder &other) {
	if (other._buffer == nullptr) {
		_buffer = nullptr;
	} else {
		_buffer = (unsigned char*) malloc(other._size);
        if (!_buffer)
            throw OutOfMemoryError(_HERE_);
		memcpy(_buffer, other._buffer, other._size);
	}

	_size = other._size;
	_len = other._len;
}

StringBuilder::StringBuilder(StringBuilder &&other) {
	_buffer = other._buffer;
	_size = other._size;
	_len = other._len;

	other._buffer = nullptr;
	other._size = 0;
	other._len = 0;
}

StringBuilder::StringBuilder(const std::string& other)
:StringBuilder(other.c_str(), other.length()) {}

StringBuilder::~StringBuilder() {
	if (_buffer)
		free(_buffer);
	_buffer = nullptr;
}

void StringBuilder::build(const char *format, ...) {
	va_list ap;
	va_start(ap, format);

	int appended = vasprintf((char**)(&_buffer), format, ap);

	if (appended >= 0) {
		_len = appended;
		_size = appended + 1;
	} else {
		if (_buffer != nullptr)
			free(_buffer);
		_buffer = (unsigned char*)malloc(16);
		if (!_buffer)
			throw OutOfMemoryError(_HERE_);
		_buffer[0] = 0;
		_len = 0;
		_size = 16;
	}

	va_end(ap);
}

StringBuilder::StringBuilder(int32_t val) {
	build("%d", val);
}

StringBuilder::StringBuilder(uint32_t val) {
	build("%u", val);
}

StringBuilder::StringBuilder(int64_t val) {
	build("%" PRId64, val);
}

StringBuilder::StringBuilder(uint64_t val) {
	build("%llu", val);
}

StringBuilder::StringBuilder(double val) {
	build("%g", val);
}

bool StringBuilder::operator ==(const StringBuilder& other) const {
	return equals(other);
}

bool StringBuilder::equals(const StringBuilder& other) const {
	if (_buffer == nullptr)
		return (other._buffer == nullptr);
	if (other._buffer == nullptr)
		return _buffer == nullptr;
	if (_buffer == other._buffer)
		return true;
	if (_len == other._len)
		return !strcmp((char*)_buffer, (char*)other._buffer);
	return false;
}

bool StringBuilder::equalsIgnoreCase(const StringBuilder& other) const {
	if (_buffer == nullptr)
		return (other._buffer == nullptr);
	if (other._buffer == nullptr)
		return _buffer == nullptr;
	if (_buffer == other._buffer)
		return true;
	if (_len == other._len)
		return !stricmp((char*)_buffer, (char*)other._buffer);
	return false;
}

bool StringBuilder::operator <(const StringBuilder& other) const {
	if (_buffer == nullptr) {
		if (other._buffer == nullptr)
			return false;
		return true;
	}
	if (other._buffer == nullptr)
		return false;
	return (strcmp((char*)_buffer, (char*)other._buffer) < 0);
}

char StringBuilder::charAt(size_t pos) const {
	return _buffer[pos];
}

ptrdiff_t StringBuilder::indexOf(char ch) const {
	const char *pos = strchr((char*)_buffer, ch);
	return (pos == nullptr ? -1 : pos - (char*)_buffer);
}

ptrdiff_t StringBuilder::indexOf(char ch, size_t fromIndex) const {
	if (fromIndex >= _len)
		return -1;

	const char *pos = strchr((char*)_buffer + fromIndex, ch);
	return (pos == nullptr ? -1 : pos - (char*)_buffer);
}

ptrdiff_t StringBuilder::indexOf(const StringBuilder& sub) const {
	const char *pos = strstr((char*)_buffer, (char*)sub._buffer);
	return (pos == nullptr ? -1 : pos - (char*)_buffer);
}

ptrdiff_t StringBuilder::indexOf(const StringBuilder& sub, size_t fromIndex) const {
	if (fromIndex >= _len)
		return -1;

	const char *pos = strstr((char*)_buffer + fromIndex, sub.c_str());
	return (pos == nullptr ? -1 : pos - (char*)_buffer);
}

ptrdiff_t StringBuilder::lastIndexOf(char ch) const {
	return lastIndexOf(ch, _len - 1);
}

ptrdiff_t StringBuilder::lastIndexOf(char ch, ptrdiff_t fromIndex) const {
	if (isNull())
		throw NullPointerException(_HERE_);

	if (fromIndex < 0)
		return -1;

	int min = 0;
	const char *v = (const char*)_buffer;

	ptrdiff_t i = (((size_t)fromIndex >= _len) ? _len - 1 : fromIndex);

	for (; i >= min ; i--) {
		if (v[i] == ch)
			return i;
	}
	return -1;
}

ptrdiff_t StringBuilder::lastIndexOf(const StringBuilder& sub) const {
	if (isNull() || sub.isNull())
		throw NullPointerException(_HERE_);
	return StringBuilder::lastIndexOf((const char*)_buffer, 0, _len, (const char*)sub._buffer, 0, sub._len, _len);
}

ptrdiff_t StringBuilder::lastIndexOf(const char *source, ptrdiff_t sourceOffset, size_t sourceCount,
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

StringBuilder StringBuilder::substring(size_t beginIndex) const {
	return substring(beginIndex, length());
}

StringBuilder StringBuilder::substring(size_t beginIndex, size_t endIndex) const {
	if (endIndex > _len)
		throw StringIndexOutOfBoundsException(_HERE_, endIndex);
	if (beginIndex > endIndex)
		throw StringIndexOutOfBoundsException(_HERE_, endIndex - beginIndex);
	size_t len = endIndex - beginIndex;
	StringBuilder res(nullptr, len + 1);
	memcpy(res._buffer, _buffer + beginIndex, len);
	res._buffer[len] = 0;
	res._len = len;
	return res;
}

StringBuilder StringBuilder::trim() const {
	size_t a = 0;
	ptrdiff_t b = _len;
	for (a = 0; (a < _len) && isspace(_buffer[a]); a++);
	for (b = _len; (b > 0) && isspace(_buffer[b - 1]); b--);

	if (b > (ptrdiff_t)a)
		return substring(a, b);
	else {
		StringBuilder res(nullptr, 1);
		res._buffer[0] = 0;
		res._len = 0;
		return res;
	}
}

StringBuilder StringBuilder::toLowerCase() const {
	if (isNull())
		return getNull();
	StringBuilder res(nullptr, _len + 1);
	for (size_t i = 0; i < _len; i++)
		res.add((char)tolower(_buffer[i]));
	return res;
}

StringBuilder StringBuilder::toUpperCase() const {
	if (isNull())
		return getNull();
	StringBuilder res(nullptr, _len + 1);
	for (size_t i = 0; i < _len; i++)
		res.add((char)toupper(_buffer[i]));
	return res;
}

bool StringBuilder::startsWith(char prefix) const {
	if (_buffer == nullptr)
		return false;
	return _buffer[0] == prefix;
}

bool StringBuilder::startsWith(const StringBuilder &prefix) const {
	return (strncmp((char*)_buffer, prefix.c_str(), prefix.length()) == 0);
}

bool StringBuilder::startsWith(const StringBuilder &prefix, ptrdiff_t offset) const {
	if (offset < 0)
		return false;
	size_t uOffset = (size_t) offset;
	if (uOffset > _len - prefix._len)
		return false;
	return (strncmp((char*)_buffer + uOffset, prefix.c_str(), prefix.length()) == 0);
}

bool StringBuilder::endsWith(char suffix) const {
	if ((_buffer == nullptr) || (_len == 0))
		return false;
	return _buffer[_len - 1] == suffix;
}

bool StringBuilder::endsWith(const StringBuilder &suffix) const {
	return (startsWith(suffix, _len - suffix.length()));
}

StringBuilder& StringBuilder::getNull() {
	return NULLSTRINGBUILDER;
}

void StringBuilder::alloc(size_t newLen) {
	grow(newLen + 1);
}

void StringBuilder::setLength(size_t newLen) {
	if (_buffer != nullptr) {
		if (newLen > _size - 1)
			newLen = _size - 1;
		_len = newLen;
		_buffer[_len] = 0;
	}
}

void StringBuilder::grow(size_t newLen) {
	size_t newSize = (_size > 0 ? _size : 1);
	while (newSize < newLen) {
		size_t oldSize = newSize;
		newSize = _STR_GROWTH_FACTOR * newSize;
		// guard against float growth factor
		if (newSize == oldSize)
			newSize++;
	}
	if (_buffer) {
		if (newSize != _size)
			_buffer = (unsigned char*) realloc(_buffer, newSize);
	} else
		_buffer = (unsigned char*) malloc(newSize);

    if (!_buffer)
        throw OutOfMemoryError(_HERE_);

	_size = newSize;
}

StringBuilder& StringBuilder::operator=(const StringBuilder& other) {
	if (this == &other)
		return *this;
	if (other._buffer == nullptr) {
		if (_buffer)
			free(_buffer);
		_buffer = nullptr; _len = 0; _size = 0;
	} else {
		grow(other._size);
		memcpy(_buffer, other._buffer, other._size);
		_size = other._size;
		_len = other._len;
	}
	return *this;	// This will allow assignments to be chained
}

StringBuilder& StringBuilder::operator=(StringBuilder &&other) {
	if (this == &other)
		return *this;
	if (_buffer)
		free(_buffer);

	_size = other._size;
	_len = other._len;
	_buffer = other._buffer;

	other._buffer = nullptr;
	other._size = 0;
	other._len = 0;
	return *this;
}

void StringBuilder::assign(const char *src, size_t len) {
	grow(len + 1);
	memcpy(_buffer, src, len);
	_len = len;
	_buffer[len] = 0;
}

void StringBuilder::assignInternal(const char *src, size_t len) {
	grow(len + 1);
	memmove(_buffer, src, len);
	_len = len;
	_buffer[len] = 0;
}

void StringBuilder::clear() {
	if (_buffer) {
		_buffer[0] = 0;
		_len = 0;
	}
}

void StringBuilder::truncate(size_t len) {
	if (_buffer) {
		if (len < _len) {
			_buffer[_len] = 0;
			_len = len;
		}
	}
}

// append char array

StringBuilder& StringBuilder::add(const char *src, ptrdiff_t len /* = -1 */) {
	if (src == nullptr)
		return *this;
	if (len < 0)
		len = strlen(src);
	if (_len + len + 1 > _size)
		grow(_len + len + 1);
	memcpy(_buffer + _len, src, len + 1);
	_len += len;
	_buffer[_len] = 0;
	return *this;
}

StringBuilder& StringBuilder::operator +=(const char* op) {
	return add(op);
}

const StringBuilder StringBuilder::operator +(const char* other) const {
	return StringBuilder(*this) += other;
}

// append StringBuilder

StringBuilder& StringBuilder::add(const StringBuilder& src) {
	if (src.isNull())
		return *this;
	ptrdiff_t len = src.length();
	if (_len + len + 1 > _size)
		grow(_len + len + 1);
	memcpy(_buffer + _len, src.c_str(), len + 1);
	_len += len;
	_buffer[_len] = 0;
	
	return *this;
}

StringBuilder& StringBuilder::operator+=(const StringBuilder& op) {
	return add(op);
}

const StringBuilder StringBuilder::operator+(const StringBuilder& other) const {
	return StringBuilder(*this) += other;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder& src) {
	return add(src);
}

// append char

StringBuilder& StringBuilder::operator<<(char c) {
	return add(c);
}

StringBuilder& StringBuilder::operator+=(char op) {
	return add(op);
}

const StringBuilder StringBuilder::operator+(char other) const {
	return StringBuilder(*this) += other;
}

// append int

StringBuilder& StringBuilder::add(int i) {
	char buffer[32];
	int n = snprintf(buffer, 31, "%d", i);
	buffer[31] = 0;
	add(buffer, n);
	return *this;
}

StringBuilder& StringBuilder::operator<<(int i) {
	return add(i);
}

StringBuilder& StringBuilder::operator+=(int op) {
	return add(op);
}

const StringBuilder StringBuilder::operator+(int other) const {
	return StringBuilder(*this) += other;
}

// append int64

StringBuilder& StringBuilder::add(int64_t i) {
	char buffer[32];
	int n = snprintf(buffer, 31, "%" PRId64, i);
	buffer[31] = 0;
	add(buffer, n);
	return *this;
}

StringBuilder& StringBuilder::operator<<(int64_t i) {
	return add(i);
}

StringBuilder& StringBuilder::operator+=(int64_t op) {
	return add(op);
}

const StringBuilder StringBuilder::operator+(int64_t other) const {
	return StringBuilder(*this) += other;
}

// append double

StringBuilder& StringBuilder::add(double d) {
	char buffer[64];
	int n = snprintf(buffer, 63, "%g", d);
	buffer[63] = 0;
	add(buffer, n);
	return *this;
}

StringBuilder& StringBuilder::operator <<(double d) {
	return add(d);
}

StringBuilder& StringBuilder::operator +=(double op) {
	return add(op);
}

const StringBuilder StringBuilder::operator +(double other) const {
	return StringBuilder(*this) += other;
}

StringBuilder& StringBuilder::addLine(const char *src, ptrdiff_t len /* = -1 */) {
	add(src, len);
	add("\n", 1);
	return *this;
}

void StringBuilder::internalAppend(const char *format, va_list ap) {
	va_list ap1;
	va_copy(ap1, ap);

	int toAppend = vsnprintf(nullptr, 0, format, ap);
	if (toAppend > 0) {
		if (_len + toAppend + 1 > _size)
			grow(_len + toAppend + 1);

		int appended = vsnprintf((char*)_buffer + _len, toAppend + 1, format, ap1);
		_len += appended;
	}
	va_end(ap1);
}

StringBuilder& StringBuilder::addFmt(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	internalAppend(format, ap);
	va_end(ap);
	return *this;
}

StringBuilder& StringBuilder::addFmtVL(const char *format, va_list ap) {
	internalAppend(format, ap);
	return *this;
}

StringBuilder& StringBuilder::addFmtLine(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	internalAppend(format, ap);
	va_end(ap);
	add("\n", 1);
	return *this;
}

NullStringBuilder NULLSTRINGBUILDER;

} // namespace
