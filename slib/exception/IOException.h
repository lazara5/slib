/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_EXCEPTION_IOEXCEPTION_H
#define H_SLIB_EXCEPTION_IOEXCEPTION_H

#include "slib/exception/Exception.h" 

namespace slib {

/**
 * Signals that a method has been invoked at an illegal or inappropriate time. In other words, 
 * the object is not in an appropriate state for the requested operation.
 */
class IOException : public Exception {
public:
	IOException(const char *where, const char *msg)
	:Exception(where, "IOException", msg) {}

	IOException(const char *where)
	:Exception(where, "IOException", "") {}
};

} // namespace slib

#endif // H_SLIB_EXCEPTION_IOEXCEPTION_H
