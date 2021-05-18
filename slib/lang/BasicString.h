/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_BASICSTRING_H
#define H_SLIB_LANG_BASICSTRING_H

#include "slib/lang/Object.h"
#include "slib/lang/StringView.h"

namespace slib {

class BasicString: virtual public Object {
public:
	TYPE_INFO(BasicString, CLASS(BasicString), INHERITS(Object));
public:
	virtual size_t length() const = 0;
	virtual const char *c_str() const = 0;

	virtual int compareTo(BasicString const& other) const;

	template <class S1, class S2>
	static bool equals(S1 const* str, S2 const* other) {
		const char *buffer = str ? str->c_str() : nullptr;
		const char *otherBuffer = other? other->c_str() : nullptr;


		if (buffer == nullptr)
			return (otherBuffer == nullptr);
		if (otherBuffer == nullptr)
			return buffer == nullptr;
		if (buffer == otherBuffer)
			return true;

		size_t len = str->length();
		size_t otherLen = other->length();
		if (len == otherLen)
			return !strcmp(buffer, otherBuffer);
		return false;
	}

	bool operator==(BasicString const& other) const {
		return equals(other);
	}

	using Object::equals;

	/**
	 * Compares this BasicString to the specified BasicString object. Returns <i>true</i>
	 * only if the argument is a BasicString object that contains the same sequence
	 * of characters as this BasicString or if both this and the other BasicString
	 * object are <i>'NULL'</i> references.
	 * @param other The BasicString object to compare this String against
	 * @return <i>true</i> if the given object represents a String object
	 *		equivalent to this String, <i>false</i> otherwise.
	 */
	virtual bool equals(BasicString const& other) const;

	UPtr<String> toUpperCase() const;
	UPtr<String> toLowerCase() const;
};

class BasicStringView : public BasicString {
public:
	TYPE_INFO(BasicStringView, CLASS(BasicStringView), INHERITS(BasicString));
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

	virtual const char *c_str() const override {
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

namespace std {
	// for using BasicString as key in unordered_map
	template<> struct hash<slib::BasicString> {
		std::size_t operator()(const slib::BasicString& s) const {
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
