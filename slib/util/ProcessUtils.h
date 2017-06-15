/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_UTIL_PROCESSUTILS_H__
#define __SLIB_UTIL_PROCESSUTILS_H__

#include "slib/exception/Exception.h"

#include <stdlib.h>

namespace slib {

class ProcessException : public Exception {
public:
	ProcessException(const char *where, const char *msg) 
	:Exception(where, "ProcessException", msg) {
	}
};

class ProcessUtils {
public:
	/** @throws ProcessException */
	static void terminateProcess(pid_t pid);
};

} // namespace

#endif

