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

	static Class const* CLASS() {
		return CHARACTERCLASS();
	}

	virtual Class const* getClass() const override {
		return CHARACTERCLASS();
	}

	virtual int32_t hashCode() const override {
		return (int32_t)_value;
	}

	char charValue() const {
		return _value;
	}

	static std::unique_ptr<String> toString(char c) {
		return std::make_unique<String>(c);
	}

	virtual std::unique_ptr<String> toString() const override {
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
};

} // namespace slib

#endif // H_SLIB_CHARACTER_H
