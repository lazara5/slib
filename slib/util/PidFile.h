/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_UTIL_PIDFILE_H__
#define __SLIB_UTIL_PIDFILE_H__

#include "slib/exception/Exception.h"
#include "slib/util/Config.h"

#include <bsd/libutil.h>

namespace slib {

class PidFileException : public Exception {
public:
	PidFileException(const char *where, const char *msg) 
	:Exception(where, msg) {}
};

class PidFile {
private:
	struct pidfh *_pfh;
private:
	static pid_t open(struct pidfh **pfh, const Config& cfg, const std::string& modeSpec = "rw-r--r--");
	static void close(struct pidfh **pfh);
	static void remove(struct pidfh **pfh);
public:
	PidFile();
	
	/** 
	 * Opens and locks pid file if possible
	 * @exception PidFileException 
	 */
	pid_t open(const Config& cfg, const std::string& modeSpec = "rw-r--r--");
	
	/** @exception PidFileException */
	void update();
	
	/** @exception PidFileException */
	void close();
	
	/** @exception PidFileException */
	void remove();
	
	/** @exception PidFileException */
	static pid_t getInstance(const Config& cfg);
};

} // namespace

#endif
