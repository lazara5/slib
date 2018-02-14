/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CONCURRENT_OPTIONALCONCURRENTQUEUE_H
#define H_SLIB_CONCURRENT_OPTIONALCONCURRENTQUEUE_H

#include "slib/util/StringUtils.h"
#include "slib/exception/Exception.h"
#include "slib/concurrent/Semaphore.h"

#include "optional/optional.hpp"

#include <sys/eventfd.h>

#include "fmt/format.h"

#include <queue>
#include <mutex>

namespace slib {

/** Multi-producer multi-consumer concurrent queue implemented via a semaphore */
template <class T>
class MPMCQueue {
private:
	std::mutex _lock;
	slib::Semaphore _dataCounter;
	std::queue<T> _queue;
public:
	MPMCQueue()
	:_dataCounter(0) {}

	void push(const T& object) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.push(object);
		}
		_dataCounter.release();
	}

	template <class... Args>
	void emplace(Args&&... args) {
		{
			std::lock_guard<std::mutex> aLock(_lock);
			_queue.emplace(std::forward<Args>(args)...);
		}
		_dataCounter.release();
	}

	std::experimental::optional<T> pop(int timeout = -1) {
		std::experimental::optional<T> value;

		bool acquired = _dataCounter.acquire(timeout);
		if (!acquired)
			return value;

		std::lock_guard<std::mutex> aLock(_lock);
		value = _queue.front();
		_queue.pop();
		return value;
	}
};

/** Multi-producer multi-consumer concurrent queue implemented via eventfd */
template <class T>
class FdMPMCQueue {
private:
	std::mutex _lock;
	int _dataCounter = -1;
	std::queue<T> _queue;
public:
	FdMPMCQueue() {
		_dataCounter = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK);
		if (_dataCounter == -1)
			throw slib::Exception(_HERE_, fmt::format("eventfd() failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
	}

	virtual ~FdMPMCQueue() {
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

	std::experimental::optional<T> pop() {
		std::experimental::optional<T> value;

		uint64_t data;
		if (read(_dataCounter, &data, sizeof(int64_t)) != sizeof(uint64_t)) {
			if (errno == EAGAIN)
				return value;
			throw slib::Exception(_HERE_, fmt::format("eventfd read failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
		}

		std::lock_guard<std::mutex> aLock(_lock);
		value = _queue.front();
		_queue.pop();
		return value;
	}

	int getFd() {
		return _dataCounter;
	}
};

} // namespace slib

#endif // H_SLIB_CONCURRENT_OPTIONALCONCURRENTQUEUE_H
