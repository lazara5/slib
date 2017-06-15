/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_EXCEPTION_ILLEGALSTATEEXCEPTION_H__
#define __SLIB_EXCEPTION_ILLEGALSTATEEXCEPTION_H__

#include "slib/exception/Exception.h" 

namespace slib {
	
/**
 * Signals that a method has been invoked at an illegal or inappropriate time. In other words, 
 * the object is not in an appropriate state for the requested operation.
 */
class IllegalStateException : public Exception {
public:
	IllegalStateException(const char *where, const char *msg) 
	: Exception(where, "IllegalStateException", msg) {
	}

	IllegalStateException(const char *where) 
	: Exception(where, "IllegalStateException", "") {
	}
};

} // namespace

#endif
