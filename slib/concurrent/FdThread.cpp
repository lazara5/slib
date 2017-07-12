#include "slib/concurrent/FdThread.h"
#include "slib/util/StringUtils.h"

#include "fmt/format.h"

#include <unistd.h>

namespace slib {

FdThread::FdThread(const std::string& name /* = "" */)
:Thread(name) {
	_fdStop = eventfd(0, EFD_SEMAPHORE);
	if (_fdStop == -1)
		throw ThreadException(_HERE_, fmt::format("eventfd() failed, errno = {}", slib::StringUtils::formatErrno()).c_str());
}

FdThread::~FdThread() {
	if (_fdStop >= 0)
		close(_fdStop);
}

int FdThread::stop() {
	_flagStop = 1;
	uint64_t data = 1;
	if (write(_flagStop, &data, sizeof(uint64_t)) != sizeof(uint64_t))
		return -1;
	return 0;
}

int FdThread::signal() {
	uint64_t data = 1;
	if (write(_flagStop, &data, sizeof(uint64_t)) != sizeof(uint64_t))
		return -1;
	return 0;
}

bool FdThread::stopRequested(long timeout, bool *signalled /*=nullptr*/) {
	if (signalled)
		*signalled = false;

	if (_flagStop)
		return true;

	if (timeout > 0) {
		fd_set rfds;
		struct timeval tout;

		FD_ZERO(&rfds);
		FD_SET(_fdStop, &rfds);
		tout.tv_sec = timeout / 1000;
		tout.tv_usec = (timeout % 1000) * 1000;

		int res = select(_fdStop + 1, &rfds, nullptr, nullptr, &tout);
		if (res <= 0) {
			// error or timeout
			return _flagStop;
		} else {
			uint64_t data;
			if (read(_fdStop, &data, sizeof(int64_t)) == sizeof(uint64_t)) {
				if (signalled)
					*signalled = true;
				return _flagStop;
			}
		}
	}

	return false;
}

} // namespace
