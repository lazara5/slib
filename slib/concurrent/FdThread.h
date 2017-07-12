/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CONCURRENT_FDTHREAD_H
#define H_SLIB_CONCURRENT_FDTHREAD_H

#include "slib/concurrent/Thread.h"

#include <sys/eventfd.h>

namespace slib {

class FdThread : public Thread {
private:
	int _fdStop;
public:
	/**
	 * Constructs a new Thread instance
	 * @param name  Thread name
	 * @throws ThreadException
	 */
	FdThread(const std::string& name = "");

	virtual ~FdThread();

	/** Requests the tread to exit from run(). Does <b>not</b> actually interrupt anything ! */
	virtual int stop();

	virtual int signal();

	/** returns a file descriptor that is readable when the thread was signalled */
	virtual int getFd() {
		return _fdStop;
	}

protected:
	/** @return true if stop() has been called for this thread. If a timeout has been specified, it waits. */
	virtual bool stopRequested(long timeout = 0, bool *signalled = nullptr);
};

} // namespace

#endif // H_SLIB_CONCURRENT_FDTHREAD_H
