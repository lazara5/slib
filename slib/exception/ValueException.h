/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_VALUEEXCEPTION_H__
#define __SLIB_VALUEEXCEPTION_H__

#include "slib/exception/Exception.h"

#include "fmt/format.h"

namespace slib {

class ValueException : public Exception {
protected:
	ValueException(const char *where, const char *className, const char *msg)
	:Exception(where, className, msg) {}
public:
	ValueException(const char *where, const char *name)
	:Exception(where, "ValueException", fmt::format("Invalid or missing value: {}", name).c_str()) {}
};

class MissingValueException : public ValueException {
public:
	MissingValueException(const char *where, const char *name)
	:ValueException(where, "MissingValueException", fmt::format("Missing value: {}", name).c_str()) {}
};

class InvalidValueException : public ValueException {
public:
	InvalidValueException(const char *where, const char *name)
	:ValueException(where, "InvalidValueException", fmt::format("Invalid value: {}", name).c_str()) {}

	InvalidValueException(const char *where, const char *name, const char *msg)
	:ValueException(where, "InvalidValueException", fmt::format("Invalid value: {} ({})", name, msg).c_str()) {}
};

} // namespace

#endif
