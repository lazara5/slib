/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_STRINGVIEW_H
#define H_SLIB_LANG_STRINGVIEW_H

#include <stddef.h>

#include "fmt/format.h"

namespace slib {

class StringView {
private:
	const char *_str;	///< internal string
	size_t _len;		///< string length
public:
	inline constexpr StringView() noexcept
	:_str(nullptr)
	,_len(0) {}

	inline constexpr StringView(const char *str, size_t len) noexcept
	:_str(str)
	,_len(len) {}

	inline constexpr StringView(StringView const& str) noexcept
	:_str(str.c_str())
	,_len(str.length()) {}

	inline constexpr const char *c_str() const noexcept {
		return _str;
	}

	inline constexpr size_t length() const noexcept {
		return _len;
	}
};

constexpr StringView operator ""_SV(const char* str, size_t len) noexcept {
	return StringView(str, len);
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, StringView const& s);

} // namespace slib

#endif // H_SLIB_LANG_STRINGVIEW_H
