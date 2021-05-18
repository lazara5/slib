/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CHARACTER_H
#define H_SLIB_CHARACTER_H

#include "slib/lang/Object.h"

namespace slib {

class Character : virtual public Object {
private:
	char _value;
public:
	Character(char value)
	:_value(value) {}

	virtual int32_t hashCode() const override {
		return (int32_t)_value;
	}

	char charValue() const {
		return _value;
	}

	static UPtr<String> toString(char c) {
		return newU<String>(c);
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
public:
	static bool isDigit(char ch) {
		return std::isdigit(ch);
	}

	static bool isUpperCase(char ch) {
		return std::isupper(ch);
	}

	static bool isLowerCase(char ch) {
		return std::islower(ch);
	}

	static int digit(char ch, int radix) {
		int result = -1;

		if (ch >= '0' && ch <= '9')
			result = ch - '0';
		else if (ch >= 'a' && ch <= 'z')
			result = ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'Z')
			result = ch - 'A' + 10;

		if (result < radix && result != -1)
			return result;

		return -1;
	}
};

} // namespace slib

#endif // H_SLIB_CHARACTER_H
