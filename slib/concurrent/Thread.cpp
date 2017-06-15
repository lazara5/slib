/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/concurrent/Thread.h"
#include "slib/util/StringUtils.h"

#include "fmt/format.h"

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

using namespace slib;

/** Java-like Thread class, using pthreads */
Thread::Thread(const std::string& name) {
	_threadId = 0;
	_flagStop = 0;

	if (name.length() < 15)
		_name = name;
	else
		_name = name.substr(0, 15);

	int res = sem_init(&_semStop, 0, 0);
	if (res < 0)
		throw ThreadException(_HERE_, fmt::format("sem_init() failed, errno='{}'", StringUtils::formatErrno()).c_str());
}

Thread::~Thread() {
	sem_destroy(&_semStop);
}

void Thread::start() {
	int res = pthread_create(&_threadId, NULL, threadProc, (void*)this);
	if (res)
		throw ThreadException(_HERE_, fmt::format("pthread_create() failed, error='{}'", StringUtils::formatErrno(res).c_str()).c_str());
	if (!_name.empty())
		pthread_setname_np(_threadId, _name.c_str());
}

void Thread::join() {
	int res = pthread_join(_threadId, NULL);
	if (res)
		throw ThreadException(_HERE_, fmt::format("pthread_join() failed, error='{}'", StringUtils::formatErrno(res).c_str()).c_str());
}

int Thread::stop() {
	_flagStop = 1;
	return sem_post(&_semStop);
}

int Thread::signal() {
	return sem_post(&_semStop);
}

bool Thread::stopRequested(long timeout, bool *signalled /*=nullptr*/) {
	int res;
	struct timeval now;
	struct timespec ts;
	long long t;	// time, in millisceconds

	if (signalled)
		*signalled = false;

	if (_flagStop)
		return true;

	if (timeout > 0) {
		gettimeofday(&now, NULL);
		t = (long long)now.tv_sec * 1000 + (long long)now.tv_usec/1000;
		t += timeout;
		t *= 1000000;					// convert from milliseconds to nanoseconds
		ts.tv_nsec = t % 1000000000;
		ts.tv_sec  = t / 1000000000;
		res = sem_timedwait(&_semStop, &ts);
		if (res == 0) {		// signalled
			if (signalled)
				*signalled = true;
			return _flagStop; 
		} else {
			if ((errno == ETIMEDOUT) || (errno == EINTR))
				return _flagStop; // timed out or interrupted
			else {
				perror("sem_timedwait error");
				return _flagStop;
			}
		}
	}

	return false;
}

void *Thread::threadProc(void *p) {
	Thread & t = *(Thread*)p;
	int res = t.run();
#ifdef _DEBUG
	fprintf(stderr, "Thread %lu exited with code %d\n", t._threadId, res);
#endif
	return nullptr;
}

