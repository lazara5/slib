/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/StringBuilder.h"
#include "slib/lang/String.h"

#include <stdarg.h>
#include <inttypes.h>

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

StringBuilder::StringBuilder(size_t capacity /* = 15 */) {
	_hash = 0;
	_buffer = (unsigned char *)malloc(capacity + 1);
	if (!_buffer)
		throw OutOfMemoryError(_HERE_);
	_buffer[0] = 0;
	_size = capacity + 1;
	_len = 0;
}

StringBuilder::StringBuilder(const char *str, ptrdiff_t len /* = -1 */) {
	_hash = 0;
	if (str == nullptr) {
		if (len < 0) {
			_buffer = nullptr;
			_len = 0;
			_size = 0;
		} else {
			_buffer = (unsigned char*)malloc((size_t)len + 1);
			if (!_buffer)
				throw OutOfMemoryError(_HERE_);
			_buffer[0] = 0;
			_size = (size_t)len + 1;
			_len = 0;
		}
	} else {
		if (len < 0)
			len = (ptrdiff_t)strlen(str);
		_buffer = (unsigned char*)malloc((size_t)len + _STR_EXTRA_ALLOC + 1);
		if (!_buffer)
			throw OutOfMemoryError(_HERE_);
		memcpy(_buffer, str, (size_t)len);
		_len = (size_t)len;
		_buffer[_len] = 0;
		_size = (size_t)len + _STR_EXTRA_ALLOC + 1;
	}
}

StringBuilder::StringBuilder(const char *str, size_t offset, size_t count) {
	_hash = 0;
	_buffer = (unsigned char*) malloc(count + _STR_EXTRA_ALLOC + 1);
	if (!_buffer)
		throw OutOfMemoryError(_HERE_);
	if (count > 0)
		memcpy(_buffer, str + offset, count);
	_len = count;
	_buffer[_len] = 0;
	_size = count + _STR_EXTRA_ALLOC + 1;
}

StringBuilder::StringBuilder(StringBuilder const& other) {
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
	_hash = other._hash;
}

StringBuilder::StringBuilder(StringBuilder &&other) {
	_buffer = other._buffer;
	_size = other._size;
	_len = other._len;
	_hash = other._hash;

	other._buffer = nullptr;
	other._size = 0;
	other._len = 0;
	other._hash = 0;
}

StringBuilder::StringBuilder(BasicString const& other)
:StringBuilder(other.data(), (ptrdiff_t)other.length()) {}

StringBuilder::StringBuilder(std::string const& other)
:StringBuilder(other.c_str(), (ptrdiff_t)other.length()) {}

StringBuilder::~StringBuilder() {
	if (_buffer)
		free(_buffer);
	_buffer = nullptr;
}

/*void StringBuilder::build(const char *format, ...) {
	_hash = 0;
	va_list ap;
	va_start(ap, format);

	int appended = vasprintf((char**)(&_buffer), format, ap);

	if (appended >= 0) {
		_len = (size_t)appended;
		_size = _len + 1;
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
}*/

bool StringBuilder::operator ==(StringBuilder const& other) const {
	return equals(other);
}

bool StringBuilder::equals(StringBuilder const& other) const {
	return BasicString::equals(this, CPtr(other));
}

bool StringBuilder::equalsIgnoreCase(StringBuilder const& other) const {
	return String::equalsIgnoreCase(this, CPtr(other));
}

bool StringBuilder::operator <(StringBuilder const& other) const {
	if (_buffer == nullptr) {
		if (other._buffer == nullptr)
			return false;
		return true;
	}
	if (other._buffer == nullptr)
		return false;
	return (strcmp((char*)_buffer, (char*)other._buffer) < 0);
}

const StringBuilder &StringBuilder::remove(size_t start, size_t end) {
	if (!_buffer)
		return *this;
	if (end > _len)
		end = _len;
	if (start > end)
		throw StringIndexOutOfBoundsException(_HERE_);
	size_t len = end - start;
	if (len > 0) {
		memmove(_buffer + start, _buffer + end + 1, _len - end + 2);
		_len -= len;
	}
	return *this;
}

char StringBuilder::charAt(size_t pos) const {
	return (char)_buffer[pos];
}

ptrdiff_t StringBuilder::indexOf(char ch) const {
	const char *pos = strchr((char*)_buffer, ch);
	return (pos == nullptr ? -1 : pos - (char*)_buffer);
}

ptrdiff_t StringBuilder::indexOf(char ch, size_t fromIndex) const {
	return String::indexOf(this, ch, fromIndex);
}

ptrdiff_t StringBuilder::indexOf(StringBuilder const& sub) const {
	return String::indexOf(this, CPtr(sub));
}

ptrdiff_t StringBuilder::indexOf(StringBuilder const& sub, size_t fromIndex) const {
	return String::indexOf(this, CPtr(sub), fromIndex);
}

ptrdiff_t StringBuilder::lastIndexOf(char ch) const {
	return String::lastIndexOf(this, ch);
}

ptrdiff_t StringBuilder::lastIndexOf(char ch, ptrdiff_t fromIndex) const {
	return String::lastIndexOf(this, ch, fromIndex);
}

ptrdiff_t StringBuilder::lastIndexOf(StringBuilder const& sub) const {
	return String::lastIndexOf(this, CPtr(sub));
}

StringBuilder StringBuilder::substring(size_t beginIndex) const {
	return substring(beginIndex, length());
}

StringBuilder StringBuilder::substring(size_t beginIndex, size_t endIndex) const {
	if (endIndex > _len)
		throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)endIndex);
	if (beginIndex > endIndex)
		throw StringIndexOutOfBoundsException(_HERE_, (ptrdiff_t)(endIndex - beginIndex));
	size_t len = endIndex - beginIndex;
	StringBuilder res(nullptr, (ptrdiff_t)(len + 1));
	memcpy(res._buffer, _buffer + beginIndex, len);
	res._buffer[len] = 0;
	res._len = len;
	return res;
}

StringBuilder StringBuilder::trim() const {
	size_t a = 0;
	ptrdiff_t b = (ptrdiff_t)_len;
	for (a = 0; (a < _len) && isspace(_buffer[a]); a++);
	for (b = (ptrdiff_t)_len; (b > 0) && isspace(_buffer[b - 1]); b--);

	if (b > (ptrdiff_t)a)
		return substring(a, (size_t)b);
	else {
		StringBuilder res(nullptr, 1);
		res._buffer[0] = 0;
		res._len = 0;
		return res;
	}
}

bool StringBuilder::startsWith(char prefix) const {
	return String::startsWith(this, prefix);
}

bool StringBuilder::startsWith(StringBuilder const& prefix) const {
	return String::startsWith(this, CPtr(prefix));
}

bool StringBuilder::startsWith(StringBuilder const& prefix, ptrdiff_t offset) const {
	return String::startsWith(this, CPtr(prefix), offset);
}

bool StringBuilder::endsWith(char suffix) const {
	return String::endsWith(this, suffix);
}

bool StringBuilder::endsWith(StringBuilder const& suffix) const {
	return String::endsWith(this, CPtr(suffix));
}

int32_t StringBuilder::hashCode() const {
	if (_buffer == nullptr)
		return 0;
	int h = _hash;
	if (h == 0 && _len > 0) {
		for (size_t i = 0; i < _len; i++)
			h = 31 * h + _buffer[i];
		_hash = h;
	}
	return h;
}

StringBuilder& StringBuilder::getNull() {
	return NULLSTRINGBUILDER;
}

void StringBuilder::alloc(size_t newLen) {
	grow(newLen + 1);
}

void StringBuilder::setLength(size_t newLen) {
	_hash = 0;
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
		newSize = (size_t)(_STR_GROWTH_FACTOR * newSize);
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

StringBuilder& StringBuilder::operator=(StringBuilder const& other) {
	if (this == &other)
		return *this;
	if (other._buffer == nullptr) {
		if (_buffer)
			free(_buffer);
		_buffer = nullptr; _len = 0; _size = 0; _hash = 0;
	} else {
		grow(other._size);
		memcpy(_buffer, other._buffer, other._size);
		_size = other._size;
		_len = other._len;
		_hash = other._hash;
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
	_hash = other._hash;
	_buffer = other._buffer;

	other._buffer = nullptr;
	other._size = 0;
	other._len = 0;
	other._hash = 0;
	return *this;
}

void StringBuilder::assign(const char *src, size_t len) {
	_hash = 0;
	grow(len + 1);
	memcpy(_buffer, src, len);
	_len = len;
	_buffer[len] = 0;
}

void StringBuilder::assignInternal(const char *src, size_t len) {
	_hash = 0;
	grow(len + 1);
	memmove(_buffer, src, len);
	_len = len;
	_buffer[len] = 0;
}

StringBuilder& StringBuilder::clear() {
	_hash = 0;
	if (_buffer) {
		_buffer[0] = 0;
		_len = 0;
	}
	return *this;
}

void StringBuilder::truncate(size_t len) {
	_hash = 0;
	if (_buffer) {
		if (len < _len) {
			_buffer[_len] = 0;
			_len = len;
		}
	}
}

char *StringBuilder::releaseBufferOwnership() {
	unsigned char *buffer = _buffer;
	_buffer = nullptr;
	_size = 0;
	_len = 0;
	_hash = 0;

	return (char*) buffer;
}

// append char array

StringBuilder& StringBuilder::add(const char *src, ptrdiff_t len /* = -1 */) {
	if (src == nullptr)
		return *this;
	_hash = 0;
	if (len < 0)
		len = (ptrdiff_t)strlen(src);
	if (_len + (size_t)len + 1 > _size)
		grow(_len + (size_t)len + 1);
	memcpy(_buffer + _len, src, (size_t)len);
	_len += (size_t)len;
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

StringBuilder& StringBuilder::add(BasicString const& other) {
	const char *otherBuffer = other.data();
	if (otherBuffer == nullptr)
		return *this;
	_hash = 0;
	size_t otherLen = other.length();
	if (_len + otherLen + 1 > _size)
		grow(_len + otherLen + 1);
	memcpy(_buffer + _len, otherBuffer, otherLen + 1);
	_len += otherLen;
	_buffer[_len] = 0;
	
	return *this;
}

StringBuilder& StringBuilder::add(ASCIICaseInsensitiveString const& src) {
	if (src.isNull())
		return *this;
	_hash = 0;
	size_t len = src.length();
	if (_len + len + 1 > _size)
		grow(_len + len + 1);
	memcpy(_buffer + _len, src.c_str(), len + 1);
	_len += len;
	_buffer[_len] = 0;

	return *this;
}

StringBuilder& StringBuilder::add(std::string const& src) {
	_hash = 0;
	size_t len = src.length();
	if (_len + len + 1 > _size)
		grow(_len + len + 1);
	memcpy(_buffer + _len, src.c_str(), len + 1);
	_len += len;
	_buffer[_len] = 0;

	return *this;
}

StringBuilder& StringBuilder::operator+=(StringBuilder const& op) {
	return add(op);
}

const StringBuilder StringBuilder::operator+(StringBuilder const& other) const {
	return StringBuilder(*this) += other;
}

StringBuilder& StringBuilder::operator<<(StringBuilder const& src) {
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
	_hash = 0;
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
	_hash = 0;
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
	_hash = 0;
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
		_hash = 0;
		if (_len + (size_t)toAppend + 1 > _size)
			grow(_len + (size_t)toAppend + 1);

		int appended = vsnprintf((char*)_buffer + _len, (size_t)toAppend + 1, format, ap1);
		_len += (size_t)appended;
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

NullStringBuilder::~NullStringBuilder() {}

NullStringBuilder NULLSTRINGBUILDER;

} // namespace
