/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/PidFile.h"
#include "slib/util/FileUtils.h"
#include "slib/util/FilenameUtils.h"
#include "slib/util/Log.h"

#include "fmt/format.h"

namespace slib {

PidFile::PidFile() {
	_pfh = nullptr;
}

pid_t PidFile::open(SPtr<Config> const& cfg, const std::string& modeSpec /*= "rw-r--r--"*/) {
	return open(&_pfh, cfg, modeSpec);
}

pid_t PidFile::open(struct pidfh **pfh, SPtr<Config> const& cfg, const std::string& modeSpec /*= "rw-r--r--"*/) {
	pid_t otherPid = 0;

	if (*pfh != nullptr) {
		// already open
		throw PidFileException(_HERE_, "Pid file already open");
	}

	SPtr<String> pidFile = cfg->getString("pidfile", newS<String>(*cfg->getAppName() + ".pid"));
	if (!FilenameUtils::isPathAbsolute(CPtr(pidFile)))
		pidFile = FilenameUtils::concat(CPtr(Log::getLogDir(cfg)), CPtr(pidFile));

	mode_t mode = 0;
	try {
		mode = FileUtils::parseModeSpec(CPtr(modeSpec));
	} catch (int err) {
		throw PidFileException(_HERE_, fmt::format("Error parsing file mode spec, errno='{}'", StringUtils::formatErrno(err)).c_str());
	}

	*pfh = pidfile_open(pidFile->c_str(), mode, &otherPid);
	if (*pfh == nullptr) {
		if (errno != EEXIST)
			throw PidFileException(_HERE_, fmt::format("Error opening pid file '{}', errno='{}'", *pidFile, StringUtils::formatErrno()).c_str());
		return otherPid;
	}

	return 0;
}

void PidFile::update() {
	if (_pfh == nullptr)
		throw PidFileException(_HERE_, "Pid file not open");
	int res = pidfile_write(_pfh);
	if (res > 0)
		throw PidFileException(_HERE_, fmt::format("Error writing to pid file, errno='{}'", StringUtils::formatErrno()).c_str());
}

void PidFile::close() {
	close(&_pfh);
}

void PidFile::close(struct pidfh **pfh) {
	if (*pfh == nullptr)
		throw PidFileException(_HERE_, "Pid file not open");
	int res = pidfile_close(*pfh);
	if (res > 0)
		throw PidFileException(_HERE_, fmt::format("Error closing pid file, errno='{}'", StringUtils::formatErrno()).c_str());
}

void PidFile::remove() {
	remove(&_pfh);
}

void PidFile::remove(struct pidfh **pfh) {
	if (*pfh == nullptr)
		throw PidFileException(_HERE_, "Pid file not open");
	int res = pidfile_remove(*pfh);
	if (res == 0)
		*pfh = nullptr;
	if (res < 0)
		throw PidFileException(_HERE_, fmt::format("Error writing to pid file, errno='{}'", StringUtils::formatErrno()).c_str());
}

pid_t PidFile::getInstance(SPtr<Config> const& cfg) {
	struct pidfh *pfh = nullptr;
	pid_t other = open(&pfh, cfg);
	if (other == 0)
		remove(&pfh);
	return other;
}

} // namespace
