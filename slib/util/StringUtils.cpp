/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/StringUtils.h"
#include "slib/lang/String.h"

namespace slib {

XMLString::XMLString(const char *str)
:StringBuilder(nullptr, str ? (ssize_t)strlen(str) : -1) {
	if (str) {
		const char *s = str;
		size_t tl = 0;
		while (*s != 0) {
			switch (*s) {
				case '<':
					tl += 4;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = '&'; _buffer[_len++] = 'l'; _buffer[_len++] = 't'; _buffer[_len++] = ';';
					break;
				case '>':
					tl += 4;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = '&'; _buffer[_len++] = 'g'; _buffer[_len++] = 't'; _buffer[_len++] = ';';
					break;
				case '\'':
					tl += 6;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = '&'; _buffer[_len++] = 'a'; _buffer[_len++] = 'p'; _buffer[_len++] = 'o'; _buffer[_len++] = 's'; _buffer[_len++] = ';';
					break;
				case '"':
					tl += 6;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = '&'; _buffer[_len++] = 'q'; _buffer[_len++] = 'u'; _buffer[_len++] = 'o'; _buffer[_len++] = 't'; _buffer[_len++] = ';';
					break;
				case '&':
					tl += 5;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = '&'; _buffer[_len++] = 'a'; _buffer[_len++] = 'm'; _buffer[_len++] = 'p'; _buffer[_len++] = ';';
					break;
				default:
					tl++;
					if (tl >= _size)
						grow(tl + 1);
					_buffer[_len++] = (unsigned char)(*s);
					break;
			}
			s++;
		}
		_buffer[_len] = 0;
	}
}

typedef enum {
	IS_APPEND, IS_DOLLAR, IS_READVAR
} IState;

SPtr<std::string> StringUtils::interpolate(std::string const& src, ValueProvider<std::string, std::string> const& vars,
										   bool ignoreUndefined) {
	const char *pattern = src.c_str();

	SPtr<std::string> result = newS<std::string>();
	IState state = IS_APPEND;
	size_t pos = 0;
	size_t dollarBegin = 0;
	while (pos < src.length()) {
		char c = pattern[pos];
		switch (state) {
			case IS_APPEND:
				if (c == '$') {
					state = IS_DOLLAR;
					dollarBegin = pos;
					break;
				}
				result->append(1, c);
				break;
			case IS_DOLLAR:
				if (c == '$') {
					state = IS_APPEND;
					result->append(1, '$');
					break;
				} else if (c == '{') {
					state = IS_READVAR;
					break;
				}
				state = IS_APPEND;
				result->append(1, '$').append(1, c);
				break;
			case IS_READVAR:
				if (c == '}') {
					std::string varName(pattern + dollarBegin + 2, pos - dollarBegin - 2);
					SPtr<std::string> value = vars.get(varName);
					if (value)
						result->append(*value);
					else {
						if (ignoreUndefined)
							result->append("${").append(varName).append("}");
						else
							throw MissingValueException(_HERE_, varName.c_str());
					}
					state = IS_APPEND;
				}
				break;
		}
		pos++;
	}
	return result;
}


} // namespace
