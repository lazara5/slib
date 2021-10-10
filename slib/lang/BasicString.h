/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_BASICSTRING_H
#define H_SLIB_LANG_BASICSTRING_H

#include "slib/lang/Object.h"
#include "slib/lang/StringView.h"

namespace slib {

class IString: virtual public Object {
public:
	TYPE_INFO(IString, CLASS(IString), INHERITS(Object));
public:
	virtual size_t length() const = 0;
	virtual const char *data() const = 0;

	virtual int compareTo(IString const& other) const;

	bool operator==(IString const& other) const {
		return equals(other);
	}

	using Object::equals;

	/**
	 * Compares this IString to the specified IString object. Returns <i>true</i>
	 * only if the argument is a IString object that contains the same sequence
	 * of characters as this IString or if both this and the other BasicString
	 * object are <i>'NULL'</i> references.
	 * @param other The BasicString object to compare this String against
	 * @return <i>true</i> if the given object represents a String object
	 *		equivalent to this String, <i>false</i> otherwise.
	 */
	virtual bool equals(IString const& other) const;

	UPtr<String> toUpperCase() const;
	UPtr<String> toLowerCase() const;
};

class BasicStringView : public IString {
public:
	TYPE_INFO(BasicStringView, CLASS(BasicStringView), INHERITS(IString));
protected:
	const char *_str;	///< internal string
	size_t _len;		///< string length
	mutable volatile int32_t _hash;
protected:
	BasicStringView()
	: _str(nullptr)
	, _len(0)
	, _hash(0) {}
public:
	BasicStringView(const char *str, size_t len) noexcept
	: _str(str)
	, _len(len)
	, _hash(0) {}

	BasicStringView(BasicStringView const& other) noexcept
	: _str(other._str)
	, _len(other._len)
	, _hash(other._hash) {}

	BasicStringView& operator=(BasicStringView const& other) {
		if (this == &other)
			return *this;
		_str = other._str;
		_len = other._len;
		_hash = other._hash;
		return *this;
	}

	virtual const char *data() const override {
		return _str;
	}

	virtual size_t length() const override {
		return _len;
	}

	virtual int32_t hashCode() const override {
		int h = _hash;
		if (h == 0) {
			if (_len > 0) {
				for (size_t i = 0; i < _len; i++)
					h = 31 * h + _str[i];
				_hash = h;
			}
		}
		return h;
	}
};

} // namespace slib

template <>
struct fmt::formatter<slib::IString> {
	const char *parse(format_parse_context& ctx) const {
		return ctx.begin();
	}

	template <typename FormatContext>
	format_context::iterator format(const slib::IString& sv, FormatContext& ctx) {
		return format_to(ctx.out(), "{:.{}}", sv.data(), sv.length());
	}
};

namespace std {
	// for using BasicString as key in unordered_map
	template<> struct hash<slib::IString> {
		std::size_t operator()(const slib::IString& s) const {
			return (size_t)s.hashCode();
		}
	};

	template<> struct hash<slib::BasicStringView> {
		std::size_t operator()(const slib::BasicStringView& s) const {
			return (size_t)s.hashCode();
		}
	};
}

#endif // H_SLIB_LANG_BASICSTRING_H
