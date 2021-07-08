/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/exception/Exception.h"

#include "fmt/format.h"

namespace slib {

Exception::Exception(const char *where, const char *className, Exception const& e)
: _where(where)
, _errorMessage(fmt::format("caused by {} [{} ({})]", e.getName(), e.getMessage(), e.where()).c_str())
, _className(className) {}

Exception::Exception(const char *where, const char *className, const char *msg, Exception const& e)
: _where(where)
, _errorMessage(fmt::format("{}, caused by {} [{} ({})]", msg, e.getName(), e.getMessage(), e.where()).c_str())
, _className(className) {}
	
ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(const char *where, size_t i)
: IndexOutOfBoundsException(where, fmt::format("Array index out of range: {}", i).c_str()) {}
} // namespace
