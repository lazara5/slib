/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CONCURRENT_CONCURRENTQUEUE_H
#define H_SLIB_CONCURRENT_CONCURRENTQUEUE_H

#include <sys/eventfd.h>
#include <unistd.h>

#include <queue>
#include <mutex>
#include <memory>

#include "fmt/format.h"

#include "slib/util/StringUtils.h"
#include "slib/exception/Exception.h"
#include "slib/concurrent/Semaphore.h"

namespace slib {

/** Multi-producer single-consumer concurrent queue implemented via eventfd */
template <class T>
class FdMPSCQueue {
private:
	std::mutex _lock;
	int _dataCounter = -1;
	std::queue<T> _queue;
public:
	FdMPSCQueue() {
		_dataCounter = eventfd(0, EFD_SEMAPHORE);
		if (_dataCounter == -1)
			throw slib::Exception(_HERE_, fmt::format("eventfd() failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	virtual ~FdMPSCQueue() {
		if (_dataCounter >= 0)
			close(_dataCounter);
	}

	void push(const T& object) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.push(object);
		}
		uint64_t data = 1;
		if (write(_dataCounter, &data, sizeof(uint64_t)) != sizeof(uint64_t))
			throw slib::Exception(_HERE_, fmt::format("eventfd write failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	template <class... Args>
	void emplace(Args&&... args) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.emplace(std::forward<Args>(args)...);
		}
		uint64_t data = 1;
		if (write(_dataCounter, &data, sizeof(uint64_t)) != sizeof(uint64_t))
			throw slib::Exception(_HERE_, fmt::format("eventfd write failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	T pop() {
		uint64_t data;
		if (read(_dataCounter, &data, sizeof(int64_t)) != sizeof(uint64_t))
			throw slib::Exception(_HERE_, fmt::format("eventfd read failed, errno = {}", slib::StringUtils::formatErrno()).c_str());

		std::lock_guard<std::mutex> aLock(_lock);
		T object = _queue.front();
		_queue.pop();
		return object;
	}

	int getFd() {
		return _dataCounter;
	}
};

/** Multi-producer multi-consumer concurrent queue of shared_ptr implemented via eventfd */
template <class T>
class ShFdMPMCQueue {
private:
	std::mutex _lock;
	int _dataCounter = -1;
	std::queue<std::shared_ptr<T> > _queue;
public:
	ShFdMPMCQueue() {
		_dataCounter = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK);
		if (_dataCounter == -1)
			throw slib::Exception(_HERE_, fmt::format("eventfd() failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	virtual ~ShFdMPMCQueue() {
		if (_dataCounter >= 0)
			close(_dataCounter);
	}

	void push(const std::shared_ptr<T>& object) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.push(object);
		}
		uint64_t data = 1;
		if (write(_dataCounter, &data, sizeof(uint64_t)) != sizeof(uint64_t))
			throw slib::Exception(_HERE_, fmt::format("eventfd write failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	std::shared_ptr<T> pop() {
		uint64_t data;
		if (read(_dataCounter, &data, sizeof(int64_t)) != sizeof(uint64_t)) {
			if (errno == EAGAIN)
				return nullptr;
			throw slib::Exception(_HERE_, fmt::format("eventfd read failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
		}

		std::lock_guard<std::mutex> aLock(_lock);
		std::shared_ptr<T> value = _queue.front();
		_queue.pop();
		return value;
	}

	int getFd() {
		return _dataCounter;
	}
};

/** Multi-producer multi-consumer concurrent queue of shared_ptr implemented via a semaphore */
template <class T>
class ShMPMCQueue {
private:
	std::mutex _lock;
	slib::Semaphore _dataCounter;
	std::queue<std::shared_ptr<T> > _queue;
public:
	ShMPMCQueue()
	:_dataCounter(0) {}

	void push(const std::shared_ptr<T>& object) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.push(object);
		}
		_dataCounter.release();
	}

	std::shared_ptr<T> pop(int timeout = -1) {
		bool acquired = _dataCounter.acquire(timeout);
		if (!acquired)
			return nullptr;

		std::lock_guard<std::mutex> aLock(_lock);
		std::shared_ptr<T> value = _queue.front();
		_queue.pop();
		return value;
	}
};

} // namespace slib

#endif // H_SLIB_CONCURRENT_CONCURRENTQUEUE_H
