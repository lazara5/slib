/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_CONCURRENT_THREAD_H__
#define __SLIB_CONCURRENT_THREAD_H__

#include "slib/exception/Exception.h"
#include "slib/util/System.h"

#include <semaphore.h>
#include <pthread.h>
#include <string>

namespace slib {

class ThreadException : public Exception {
public:
	ThreadException(const char *where, const char *msg)
	:Exception(where, "ThreadException", msg) {}
};

/**
 * Somewhat Java-like thread class, using pthreads
 */
class Thread {
private:
	std::string _name;

	pthread_t _threadId;
	sem_t _semStop;	// for signalling the stop flag has been set
	AtomicFlag _flagStop;
private:
	/** internal procedure, calls run() */
	static void *threadProc(void *p);

	/** checks result code returned by a pthread function */
	static int checkResult(int code, const char * str); 
public:
	/**
	 * Constructs a new Thread instance
	 * @param name  Thread name
	 * @throws ThreadException
	 */
	Thread(const std::string& name = "");

	virtual ~Thread();

	/** implement this to do useful work */
	virtual int run() = 0; 

	/** 
	 * Starts the thread.
	 * @throws ThreadException
	 */
	virtual void start();

	/** 
	 * Waits for the thread to exit.
	 * @throws ThreadException
	 */
	void join();

	/** Requests the tread to exit from run(). Does <b>not</b> actually interrupt anything ! */
	virtual int stop(); 

	virtual int signal();

	virtual pthread_t getId() {
		return _threadId;
	}
protected:
	/** @return true if stop() has been called for this thread. If a timeout has been specified, it waits. */
	virtual bool stopRequested(long timeout = 0, bool *signalled = nullptr);
};

} // namespace

#endif

