/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_UTIL_SYSTEM_H__
#define __SLIB_UTIL_SYSTEM_H__

#include "slib/libstdinclude.h"

#include <stdio.h>
#if __STDC_NO_ATOMICS__ != 1
#include <atomic>
#endif

#if __STDC_NO_ATOMICS__ == 1
	typedef volatile sig_atomic_t AtomicFlag;
#elif ATOMIC_INT_LOCK_FREE == 0 || ATOMIC_INT_LOCK_FREE == 1
	typedef volatile sig_atomic_t AtomicFlag;
#else
	typedef std::atomic_int AtomicFlag;
#endif

namespace slib {

class System {
public:
	static int64_t currentTimeMillis();
};

} // namespace

#endif
