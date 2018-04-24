/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/String.h"
#include "slib/StringBuilder.h"

#include <sstream>
#include <string>

using std::ptrdiff_t;

namespace slib {

String::String(std::string const& str)
:_str(str)
,_hash(0) {}

String::~String() {}

ASCIICaseInsensitiveString::ASCIICaseInsensitiveString() {
	_hash = 0;
	_buffer = (unsigned char*)malloc(1);
	_buffer[0] = 0;
	_len = 0;
}

ASCIICaseInsensitiveString::ASCIICaseInsensitiveString(const char *str, ptrdiff_t len /* = -1 */)
:_hash(0) {
	if (str == nullptr) {
		if (len < 0) {
			_buffer = nullptr;
			_len = 0;
		} else {
			_buffer = (unsigned char*)malloc(1);
			_buffer[0] = 0;
			_len = 0;
		}
	} else {
		if (len < 0)
			len = (ptrdiff_t)strlen(str);
		_buffer = (unsigned char*)malloc((size_t)len + 1);
		memcpy(_buffer, str, (size_t)len);
		_len = (size_t)len;
		_buffer[_len] = 0;
	}
}

ASCIICaseInsensitiveString::ASCIICaseInsensitiveString(const char *str, size_t offset, size_t count)
:_hash(0) {
	_buffer = (unsigned char*) malloc(count + 1);
	if (count > 0)
		memcpy(_buffer, str + offset, count);
	_len = count;
	_buffer[_len] = 0;
}

ASCIICaseInsensitiveString::ASCIICaseInsensitiveString(const ASCIICaseInsensitiveString &other) {
	if (other._buffer == nullptr) {
		_buffer = nullptr;
	} else {
		_buffer = (unsigned char*) malloc(other._len + 1);
		memcpy(_buffer, other._buffer, other._len + 1);
	}

	_len = other._len;
	_hash = other._hash;
}

ASCIICaseInsensitiveString::ASCIICaseInsensitiveString(const std::string& other)
:ASCIICaseInsensitiveString(other.c_str(), (ptrdiff_t)other.length()) {}

ASCIICaseInsensitiveString::~ASCIICaseInsensitiveString() {
	if (_buffer)
		free(_buffer);
	_buffer = nullptr;
}

bool ASCIICaseInsensitiveString::operator==(const ASCIICaseInsensitiveString& other) const {
	return equals(other);
}

bool ASCIICaseInsensitiveString::equals(const ASCIICaseInsensitiveString& other) const {
	return equalsIgnoreCase(other);
}

bool ASCIICaseInsensitiveString::equalsIgnoreCase(const ASCIICaseInsensitiveString& other) const {
	if (_buffer == nullptr)
		return (other._buffer == nullptr);
	if (other._buffer == nullptr)
		return _buffer == nullptr;
	if (_buffer == other._buffer)
		return true;
	if (_len == other._len)
		return !strcasecmp((char*)_buffer, (char*)other._buffer);
	return false;
}

int32_t ASCIICaseInsensitiveString::hashCode() const {
	if (_buffer == nullptr)
		return 0;
	int h = _hash;
	if (h == 0 && _len > 0) {
		for (size_t i = 0; i < _len; i++)
			h = 31 * h + _toLower[_buffer[i]];
		_hash = h;
	}
	return h;
}

ASCIICaseInsensitiveString& ASCIICaseInsensitiveString::getNull() {
	return NULLASCIICISTRING;
}

ASCIICaseInsensitiveString NULLASCIICISTRING(nullptr);

const unsigned char ASCIICaseInsensitiveString::_toLower[] = {

	0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,

	16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,

	32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,

	48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,

	64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,

	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91,  92,  93,  94,  95,

	96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,

	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,

	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,

	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,

	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,

	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,

	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,

	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,

	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,

	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255

};

} // namespace

