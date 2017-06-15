/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_EXCEPTION_NUMERICEXCEPTIONS_H__
#define __SLIB_EXCEPTION_NUMERICEXCEPTIONS_H__

#include "slib/exception/Exception.h" 

namespace slib {

/**
 * Thrown to indicate that the application has attempted to convert 
 * a string to one of the numeric types, but that the string does not 
 * have the appropriate format.
 */
class NumberFormatException : public Exception {
public:
	NumberFormatException(const char *where) : Exception(where, "NumberFormatException", "") {}

	NumberFormatException(const char *where, const char *msg) : Exception(where, "NumberFormatException", msg) {}
protected:
	NumberFormatException(const char *where, const char *name, const char *msg) : Exception(where, "NumberFormatException", msg) {}
};

class NumericOverflowException : public NumberFormatException {
public:
	NumericOverflowException(const char *where) : NumberFormatException(where, "NumericOverflowException", "") {}

	NumericOverflowException(const char *where, const char *msg) : NumberFormatException(where, "NumericOverflowException", msg) {}
};

} // namespace

#endif

