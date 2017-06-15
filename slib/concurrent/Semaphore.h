/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_CONCURRENT_LINUX_SEMAPHORE_H__
#define __SLIB_CONCURRENT_LINUX_SEMAPHORE_H__

#include <semaphore.h>
#include <limits.h>

namespace slib {

class Semaphore {
private:
	sem_t _sem;

public:
	Semaphore(long initialCount = 0, long maxCount = LONG_MAX);

	~Semaphore();

	void acquire();
	bool tryAcquire();
	bool acquire(int timeout);
	
	void release(long count = 1); 
};

} // namespace

#endif
