/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_EXCEPTION_ILLEGALACCESSEXCEPTION_H
#define H_SLIB_EXCEPTION_ILLEGALACCESSEXCEPTION_H

#include "slib/exception/Exception.h"

namespace slib {

/**
 * Signals when an application tries to reflectively set or get a field, or invoke a method,
 * but the underlying field is not accessible.
 */
class IllegalAccessException : public Exception {
public:
	IllegalAccessException(const char *where, const char *msg)
	: Exception(where, "IllegalAccessException", msg) {
	}

	IllegalAccessException(const char *where)
	: Exception(where, "IllegalAccessException", "") {
	}
};

} // namespace slib

#endif // H_SLIB_EXCEPTION_ILLEGALACCESSEXCEPTION_H
