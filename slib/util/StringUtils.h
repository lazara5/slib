/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_STRINGUTILS_H
#define H_SLIB_UTIL_STRINGUTILS_H

#include "slib/exception/Exception.h"
#include "slib/collections/Map.h"
#include "slib/lang/StringBuilder.h"
#include "slib/util/TemplateUtils.h"

#include "fmt/format.h"

#include <string>
#include <unordered_map>
#include <functional>

#include <string.h>

namespace slib {

/** Convenience class for building XML-escaped strings */
class XMLString : public StringBuilder {
public:
	XMLString(const char *str);

	XMLString(const StringBuilder& s)
	:XMLString(s.c_str()) {}

	XMLString(const std::string& s)
	:XMLString(s.c_str()) {}
};

class StringUtils {
public:
	static std::string formatException(std::string const& msg, Exception const& e) {
		return fmt::format("{} [{}: {} ({})]", msg, e.getName(), e.getMessage(), e.where());
	}

	static std::string formatErrno(int err) {
		char buffer[1024];
		buffer[0] = 0;

		const char *msg = strerror_r(err, buffer, sizeof(buffer));
		return fmt::format("{:d} ({})", err, msg);
	}

	static std::string formatErrno() {
		return formatErrno(errno);
	}

	static bool isBlank(const char *str, size_t len) {
		if ((!str) || (len == 0))
			return true;
		for (size_t i = 0; i < len; i++) {
			if (!std::isspace(str[i]))
				return false;
		}
		return true;
	}

	template <class S>
	static bool isBlank(S const& str) {
		return isBlank(strData(CPtr(str)), strLen(CPtr(str)));
	}

	template <class S>
	static bool isEmpty(S const* str) {
		const char *buffer = strData(str);
		size_t len = strLen(str);
		if ((!buffer) || (len == 0))
			return true;
		return false;
	}

	static SPtr<std::string> interpolate(std::string const& src,
										 ValueProvider<std::string, std::string> const& vars, bool ignoreUndefined);
};

class StringSplitIterator {
private:
	const char _delim;
	const char *_ptr;
	size_t _len;
	const char *_tokenStart;
	size_t _tokenLen;
	bool _init;
private:
	void computeNext() {
		if (!_ptr) {
			_tokenStart = nullptr;
			return;
		}
		const char *nextDelim = (const char *)memchr(_ptr, _delim, _len);
		if (nextDelim)
			_len -= (size_t)(nextDelim - _ptr + 1);
		_tokenStart = _ptr;
		if (!nextDelim)
			_tokenLen = _len;
		else
			_tokenLen = (size_t)(nextDelim - _ptr);
		_ptr = nextDelim ? nextDelim + 1 : nullptr;

		_init = true;
	}
public:
	template <class S>
	StringSplitIterator(S const* str, char delim)
	: _delim(delim)
	, _ptr(strData(str))
	, _len(strLen(str))
	, _init(false) {}

	bool hasNext() {
		if (!_init)
			computeNext();
		return _tokenStart != nullptr;
	}

	BasicStringView next() {
		if (!hasNext())
			throw NoSuchElementException(_HERE_);
		const char *tokenStart = _tokenStart;
		size_t tokenLen = _tokenLen;
		computeNext();
		return BasicStringView(tokenStart, tokenLen);
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_STRINGUTILS_H
