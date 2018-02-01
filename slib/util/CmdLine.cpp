/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/CmdLine.h"
#include "slib/exception/IOException.h"

#include "fmt/format.h"

namespace slib {

CmdLine::CmdLine(const char *description)
:_cmd(description, ' ', "", false)
,_helpArg("h", "help", "Print help and exit", _cmd, false)
,_versionArg("v", "version", "Print version and exit", _cmd, false)
,_consoleArg("c", "console", "Run in console", _cmd, false) {
	_action = CMD_ACTION_NONE;
}

void CmdLine::parse(int argc, char **argv) {
	try {
		_cmd.parse(argc, argv);
	} catch (TCLAP::ArgException const& e) {
		throw InitException(_HERE_, fmt::format("cmd line parse error: {}, arg: {}", e.error(), e.argId()).c_str());
	}

	if (_helpArg.getValue())
		setAction(CMD_ACTION_HELP);

	if (_versionArg.getValue())
		setAction(CMD_ACTION_VERSION);
}

} // namespace
