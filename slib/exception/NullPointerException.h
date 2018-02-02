/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_EXCEPTION_NULLPOINTEREXCEPTION_H
#define H_SLIB_EXCEPTION_NULLPOINTEREXCEPTION_H

#include "slib/exception/Exception.h"

namespace slib {

/**
 * Thrown when an application attempts to use a <i>'NULL'</i> reference or
 * a <i><b>NULL</b></i> pointer when a non-<i><b>NULL</b></i> reference/pointer
 * is required
 */
class NullPointerException : public Exception {
public:
	NullPointerException(const char *where, const char *msg) 
	: Exception(where, "NullPointerException", msg) {
	}

	NullPointerException(const char *where) 
	: Exception(where, "NullPointerException", "") {
	}
};

} // namespace slib

#endif // H_SLIB_EXCEPTION_NULLPOINTEREXCEPTION_H

