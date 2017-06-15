/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_EXCEPTION_UNSUPPORTEDOPERATIONEXCEPTION_H__
#define __SLIB_EXCEPTION_UNSUPPORTEDOPERATIONEXCEPTION_H__

#include "slib/exception/Exception.h"

namespace slib {

/** Thrown to indicate that the requested operation is not supported. */
class UnsupportedOperationException : public Exception {
	public:
	UnsupportedOperationException(const char *where, const char *msg)
	:Exception(where, "UnsupportedOperationException", msg) {}

	UnsupportedOperationException(const char *where)
	:Exception(where, "UnsupportedOperationException", "") {}
};

} // namespace

#endif

