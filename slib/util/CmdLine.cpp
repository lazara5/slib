/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/CmdLine.h"

namespace slib {

CmdLine::CmdLine(const char *description)
:_cmd(description, ' ', "", false)
,_helpArg("h", "help", "Print help and exit", _cmd, false)
,_versionArg("v", "version", "Print version and exit", _cmd, false)
,_consoleArg("c", "console", "Run in console", _cmd, false) {
	_action = CMD_ACTION_NONE;
}

void CmdLine::parse(int argc, char **argv) {
	_cmd.parse(argc, argv);

	if (_helpArg.getValue())
		setAction(CMD_ACTION_HELP);

	if (_versionArg.getValue())
		setAction(CMD_ACTION_VERSION);
}

} // namespace
