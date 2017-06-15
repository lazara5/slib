/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/concurrent/Semaphore.h"
#include "slib/util/StringUtils.h"
#include "slib/exception/Exception.h"

#include "fmt/format.h"

namespace slib {
	
Semaphore::Semaphore(long initialCount /*= 0*/, long maxCount /*= LONG_MAX*/) {
	memset(&_sem, 0, sizeof(_sem));
	if (sem_init(&_sem, 0, initialCount) != 0)
		throw Exception(_HERE_, fmt::format("sem_init() failed, errno = {}", StringUtils::formatErrno()).c_str());
}

Semaphore::~Semaphore() {
	sem_destroy(&_sem);
}

void Semaphore::acquire() {
	while (sem_wait(&_sem)) {
		if (errno == EINTR)
			errno = 0;
		else
			throw Exception(_HERE_, fmt::format("sem_wait() failed, errno = {}", StringUtils::formatErrno()).c_str());
	}
}

bool Semaphore::tryAcquire() {
	while (sem_trywait(&_sem)) {
		if (errno == EINTR) 
			errno = 0;
		else if (errno == EAGAIN)
			return false;
		else
			throw Exception(_HERE_, fmt::format("sem_trywait() failed, errno = {}", StringUtils::formatErrno()).c_str());
	}
	return true;
}

void timespecAddMs(struct timespec *ts, long ms) {
	int sec = ms / 1000;
	ms = ms - sec * 1000;

	ts->tv_nsec += ms * 1000000;

	ts->tv_sec += ts->tv_nsec / 1000000000 + sec;
	ts->tv_nsec = ts->tv_nsec % 1000000000;
}

bool Semaphore::acquire(int timeout) {
	if (timeout == -1) {
		acquire();
		return true;
	} else if (timeout == 0)
		return tryAcquire();
		
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		throw Exception(_HERE_, fmt::format("clock_gettime() failed, errno = {}", StringUtils::formatErrno()).c_str());

	timespecAddMs(&ts, timeout);

	while (sem_timedwait(&_sem, &ts)) {
		if (errno == EINTR) 
			errno = 0;
		else if (errno == ETIMEDOUT)
			return false;
		else
			throw Exception(_HERE_, fmt::format("sem_timedwait() failed, errno = {}", StringUtils::formatErrno()).c_str());
	}
	return true;
}

void Semaphore::release(long count /*= 1*/) {
	if (count == 1) {
		if (sem_post(&_sem) < 0)
			throw Exception(_HERE_, fmt::format("sem_post() failed, errno = {}", StringUtils::formatErrno()).c_str());
	} else {
		if (count > 1) {
			while(count > 0) {
				if (sem_post(&_sem) < 0)
					throw Exception(_HERE_, fmt::format("sem_post() failed, errno = {}", StringUtils::formatErrno()).c_str());
				count--;
			}
		}
	}
}

} // namespace
