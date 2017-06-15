/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/Config.h"
#include "slib/util/FileUtils.h"
#include "slib/String.h"
#include "slib/ArrayList.h"

#include <iostream>
#include <fstream>
#include <string>

#define PROCSELFEXE "/proc/self/exe"

namespace slib {
	
Config::Config(const std::string& confFileName, const std::string& appName)
:_confFileName(confFileName)
,_appName(appName) {
}

std::string searchConfigFile(List<std::string> const& configDirs, const std::string& configFile) {
	ConstIterator<std::string> i = configDirs.constIterator();
	while (i.hasNext()) {
		std::string const& dir = i.next();
		std::string fileName = FileUtils::buildPath(dir, configFile);
		if (!access(fileName.c_str(), 0))
			return dir;
	}

	return "";
}

bool dmp(void *data, const std::string& k, const std::string& v) {
	printf("%s = %s\n", k.c_str(), v.c_str());
	return true;
}

/** @throws InitException */
void Config::init() {
	char pathBuf[1024] = "";
	struct stat s;

	if (stat(PROCSELFEXE, &s) < 0)
		perror("stat " PROCSELFEXE);
	else if (readlink(PROCSELFEXE, pathBuf, sizeof(pathBuf)) < 0) {
		perror("readlink " PROCSELFEXE);
		pathBuf[0] = '\0';
	}

	if (pathBuf[0] == '\0') {
		// cannot get exe dir, attempt to use current directory
		if (getcwd(pathBuf, sizeof(pathBuf)) == NULL)
			throw InitException(_HERE_, fmt::format("getcwd() failed, errno={:d}", errno).c_str());
	} else {
		char *r = strrchr(pathBuf, '/');
		if (r)
			*r = '\0';
	}
	char *r = strrchr(pathBuf, '/');
		if (r)
			*r = '\0';

	_rootDir = pathBuf;

	ArrayList<std::string> confList;
	confList.add("/etc");
	confList.add(FileUtils::buildPath(_rootDir, "conf"));
	onBeforeSearch(confList);

	std::string configDir = searchConfigFile(confList, _confFileName);
	if (configDir.length() < 1)
		throw InitException(_HERE_, fmt::format("Could not locate config file '{}'", _confFileName).c_str());

	_confDir = configDir;

	openConfigFile();
	//_vars.forEach(&dmp, nullptr);
	readConfig();
}

void Config::readConfig() {
	// read home dir
	_homeDir = getValue("home", _rootDir);
	
	// read logdir
	_logDir = getValue("logdir", "");
	if (_logDir.empty())
		_logDir = FileUtils::buildPath(_homeDir, "log");
}
	
void Config::openConfigFile() {
	std::string configFile = FileUtils::buildPath(_confDir, _confFileName);
	
	std::ifstream iniFile(configFile.c_str());
	if (!iniFile.is_open())
		throw InitException(_HERE_, fmt::format("Cannot open config file '{}', errno='{}'", configFile, StringUtils::formatErrno()).c_str());
		
	std::string line;
	while (getline(iniFile, line)) {
		std::string iniLine = String::trim(line);
		if (String::startsWith(iniLine, ';') || String::startsWith(iniLine, '#'))
			continue;
		std::ptrdiff_t eqPos;
		if ((eqPos = String::indexOf(iniLine, '=')) > 0) {
			std::string name = String::trim(String::substring(iniLine, 0, eqPos));
			std::string rawValue = String::trim(String::substring(iniLine, eqPos + 1));
			if (String::endsWith(name, ']')) {
				ptrdiff_t openBracket = String::lastIndexOf(name, '[');
				if (openBracket > 0) {
					std::string mapName = String::trim(String::substring(name, 0, openBracket));
					std::string mapEntry = String::trim(String::substring(name, openBracket + 1, name.length() - 1));
					if ((!mapName.empty()) && (!mapEntry.empty())) {
						bool sunk = _vars.sink(mapName, mapEntry, interpolate(rawValue));
						if (sunk)
							continue;
					}
				}
			}
			_vars.set(name, interpolate(rawValue));
		}
	}
	if (iniFile.bad())
		throw InitException(_HERE_, fmt::format("Cannot read config file '{}', errno='{}'", configFile, StringUtils::formatErrno()).c_str());

	iniFile.close();
}

void Config::shutdown() {
	Log::shutdown();
}

} // namespace
