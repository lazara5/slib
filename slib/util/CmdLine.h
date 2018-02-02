/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_CMDLINE_H
#define H_SLIB_UTIL_CMDLINE_H

#include <tclap/CmdLine.h>

namespace slib {

enum GenericAction {
	CMD_ACTION_NONE = 0,
	CMD_ACTION_HELP,
	CMD_ACTION_VERSION,
	CMD_ACTION_USER_DEFINED
};

class CmdLine {
private:
	TCLAP::CmdLine _cmd;
	int _action;

	TCLAP::SwitchArg _helpArg;
	TCLAP::SwitchArg _versionArg;
	TCLAP::SwitchArg _consoleArg;
public:
	CmdLine(const char *description);
	virtual ~CmdLine() {}

	/** @throws InitException */
	virtual void parse(int argc, char **argv);

	void printHelp() {
		_cmd.getOutput()->usage(_cmd);
	}
protected:
	void setAction(int action) {
		_action = action;
	}

	TCLAP::CmdLine& getCmd() {
		return _cmd;
	}
public:
	bool isConsole() {
		return _consoleArg.getValue();
	}

	int getAction() {
		return _action;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_CMDLINE_H
