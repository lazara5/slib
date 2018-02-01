/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/StringUtils.h"
#include "slib/String.h"

namespace slib {

std::string ValueMap::getVar(const std::string& name) const {
	StringMapConstIter val = _vars.find(name);
	if (val == _vars.end())
		throw MissingValueException(_HERE_, name.c_str());
	else
		return val->second;
}
	
std::string ValueMap::get(const std::string& name) {
	StringMapConstIter val = _vars.find(name);
	if (val == _vars.end()) {
		std::ptrdiff_t dotPos;
		if ((dotPos = String::lastIndexOf(name, '.')) > 0) {
			std::string providerName = String::substring(name, 0, dotPos);
			std::string propertyName = String::substring(name, dotPos + 1);
			SourceMapConstIter provider = _sources.find(providerName);
			if (provider == _sources.end())
				throw MissingValueException(_HERE_, name.c_str());
			else
				return provider->second->getProperty(propertyName);
		} else
			throw MissingValueException(_HERE_, name.c_str());
	} else
		return val->second;
}


bool ValueMap::sink(const std::string& sinkName, const std::string& name, const std::string& value) {
	SinkMapConstIter sink = _sinks.find(sinkName);
	if (sink == _sinks.end())
		return false;
	sink->second(name, value);
	return true;
}

XMLString::XMLString(const char *str)
:StringBuilder(nullptr, str ? strlen(str) : -1) {
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
					_buffer[_len++] = *s;
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

std::string StringUtils::interpolate(std::string const& src, ValueMap& vars) {
	const char *pattern = src.c_str();
	
	std::string result;
	IState state = IS_APPEND;
	size_t pos = 0;
	int dollarBegin = 0;
	while (pos < src.length()) {
		char c = pattern[pos];
		switch (state) {
			case IS_APPEND:
				if (c == '$') {
					state = IS_DOLLAR;
					dollarBegin = pos;
					break;
				}
				result.append(1, c);
				break;
			case IS_DOLLAR:
				if (c == '$') {
					state = IS_APPEND;
					result.append(1, '$');
					break;
				} else if (c == '{') {
					state = IS_READVAR;
					break;
				}
				state = IS_APPEND;
				result.append(1, '$').append(1, c);
				break;
			case IS_READVAR:
				if (c == '}') {
					std::string varName(pattern + dollarBegin + 2, pos - dollarBegin - 2);
					result.append(vars.get(varName));
					state = IS_APPEND;
				}
				break;
		}
		pos++;
	}
	return result;
}

} // namespace
