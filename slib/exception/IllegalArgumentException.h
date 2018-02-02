/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_EXCEPTION_ILLEGALARGUMENTEXCEPTION_H
#define H_SLIB_EXCEPTION_ILLEGALARGUMENTEXCEPTION_H

#include "slib/exception/Exception.h" 

namespace slib {

/**
 * Thrown to indicate that a method has been passed an illegal or
 * inappropriate argument.
 */
class IllegalArgumentException : public Exception {
	public:
	IllegalArgumentException(const char *where, const char *msg) 
	: Exception(where, "IllegalArgumentException", msg) {
	}

	IllegalArgumentException(const char *where) 
	: Exception(where, "IllegalArgumentException", "") {
	}
};
	
} // namespace slib

#endif // H_SLIB_EXCEPTION_ILLEGALARGUMENTEXCEPTION_H
