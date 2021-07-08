/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_STRINGVIEW_H
#define H_SLIB_LANG_STRINGVIEW_H

#include "slib/util/TemplateUtils.h"

#include <stddef.h>

#include "fmt/format.h"

namespace slib {

// ---- Helpers for generic string API ----
template <class S>
const char *cStr(S const* str) {
	return str ? str->c_str() : nullptr;
}

const char *cStr(const char *str);

template <class S>
const char *strData(S const* str) {
	return str ? str->data() : nullptr;
}

const char *strData(const char *str);

template <class S>
size_t strLen(S const* str) {
	return str? str->length() : 0;
}

size_t strLen(const char *str);

class StringView {
private:
	const char *_str;		///< internal string
	const size_t _len;		///< string length
	const uint32_t _hash;	///< hash code
public:
	/** FNV-1a 32bit hashing algorithm */
	constexpr uint32_t fnv1a_32(const char *str, size_t count) const {
		return ((count ? fnv1a_32(str, count - 1) : 2166136261u) ^ str[count]) * 16777619u;
	}

	template <class S1, class S2>
	static bool equals(S1 const& str, S2 const& other) {
		const char *buffer = strData(CPtr(str));
		const char *otherBuffer = strData(CPtr(other));

		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return buffer == nullptr;
		if (buffer == otherBuffer)
			return true;

		size_t len = strLen(CPtr(str));
		size_t otherLen = strLen(CPtr(other));
		if (len == otherLen)
			return !memcmp(buffer, otherBuffer, len);
		return false;
	}
public:
	inline constexpr StringView() noexcept
	: _str(nullptr)
	, _len(0)
	, _hash(0) {}

	inline StringView(const char *str) noexcept
	: _str(str)
	, _len(strlen(str))
	, _hash(fnv1a_32(str, _len)) {}

	inline constexpr StringView(const char *str, size_t len) noexcept
	: _str(str)
	, _len(len)
	, _hash(fnv1a_32(str, len)) {}

	inline constexpr StringView(StringView const& other) noexcept
	: _str(other._str)
	, _len(other._len)
	, _hash(other._hash) {}

	inline constexpr const char *data() const noexcept {
		return _str;
	}

	inline constexpr size_t length() const noexcept {
		return _len;
	}

	inline int32_t hashCode() const noexcept {
		return _hash;
	}

	bool operator==(StringView const& other) const noexcept {
		return equals(this, &other);
	}
};

constexpr StringView operator ""_SV(const char* str, size_t len) noexcept {
	return StringView(str, len);
}

} // namespace slib

template <>
struct fmt::formatter<slib::StringView> {
	const char *parse(format_parse_context& ctx) const {
		return ctx.begin();
	}

	template <typename FormatContext>
	format_context::iterator format(const slib::StringView& sv, FormatContext& ctx) {
		return format_to(ctx.out(), "{:.{}}", sv.data(), sv.length());
	}
};

namespace std {
	// for using StringView as key in unordered_map
	template<> struct hash<slib::StringView> {
		std::size_t operator()(const slib::StringView& s) const {
			return (size_t)s.hashCode();
		}
	};
}

#endif // H_SLIB_LANG_STRINGVIEW_H
