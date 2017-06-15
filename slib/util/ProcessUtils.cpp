/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/ProcessUtils.h"
#include "slib/util/StringUtils.h"

#include "fmt/format.h"

#include <signal.h>
#include <string.h>

namespace slib {

void ProcessUtils::terminateProcess(pid_t pid) {
	int res = kill(pid, SIGTERM);
	if (res < 0)
		throw ProcessException(_HERE_, fmt::format("kill() failed, errno='{}'", StringUtils::formatErrno()).c_str());
}

} // namespace
