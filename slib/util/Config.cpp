/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/Config.h"
#include "slib/util/FileUtils.h"
#include "slib/String.h"
#include "slib/collections/ArrayList.h"
#include "slib/io/FileInputStream.h"

#include <iostream>
#include <fstream>
#include <string>

#define PROCSELFEXE "/proc/self/exe"

namespace slib {

std::shared_ptr<std::string> ConfigProcessor::processLine(const std::string& name,
														  const std::string& rawProperty) {
	std::shared_ptr<std::string> value = StringUtils::interpolate(rawProperty, *this, false);
	if (String::startsWith(name, '@')) {
		if (!_vars)
			_vars = std::make_unique<VarMap>();
		_vars->put(String::substring(name, 1), value);
		return nullptr;
	} else if (String::endsWith(name, ']')) {
		ptrdiff_t openBracket = String::lastIndexOf(name, '[');
		if (openBracket > 0) {
			std::string mapName = String::trim(String::substring(name, 0, (size_t)openBracket));
			std::string mapEntry = String::trim(String::substring(name, (size_t)openBracket + 1, name.length() - 1));
			if ((!mapName.empty()) && (!mapEntry.empty())) {
				bool sunk = sink(mapName, mapEntry, *value);
				if (sunk)
					return nullptr;
			}
		}
	}
	return value;
}

bool ConfigProcessor::sink(const std::string& sinkName, const std::string& name, const std::string& value) {
	SinkMapConstIter sink = _sinks.find(sinkName);
	if (sink == _sinks.end())
		return false;
	sink->second(name, value);
	return true;
}

std::shared_ptr<std::string> ConfigProcessor::get(std::string const& name) const {
	std::shared_ptr<std::string> value = _props.get(name);
	if ((!value) && _vars)
		value = _vars->get(name);
	if ((!value) && _sources) {
		std::ptrdiff_t dotPos;
		if ((dotPos = String::lastIndexOf(name, '.')) > 0) {
			std::string providerName = String::substring(name, 0, (size_t)dotPos);
			std::string propertyName = String::substring(name, (size_t)dotPos + 1);
			SourceMapConstIter provider = _sources->find(providerName);
			if (provider != _sources->end())
				return provider->second->get(propertyName);
		}
	}
	return value;
}

bool ConfigProcessor::containsKey(std::string const& name) const {
	if (_props.containsKey(name))
		return true;
	if (_vars && _vars->containsKey(name))
		return true;
	if (_sources) {
		std::ptrdiff_t dotPos;
		if ((dotPos = String::lastIndexOf(name, '.')) > 0) {
			std::string providerName = String::substring(name, 0, (size_t)dotPos);
			std::string propertyName = String::substring(name, (size_t)dotPos + 1);
			SourceMapConstIter provider = _sources->find(providerName);
			if ((provider != _sources->end()) && (provider->second->containsKey(propertyName)))
				return true;
		}
	}
	return false;
}

std::shared_ptr<std::string> SimpleConfigProcessor::processLine(const std::string& name,
																const std::string& rawProperty) {
	std::shared_ptr<std::string> value = StringUtils::interpolate(rawProperty, *this, true);
	if (String::startsWith(name, '@')) {
		if (!_vars)
			_vars = std::make_unique<VarMap>();
		_vars->put(String::substring(name, 1), value);
		return nullptr;
	}
	return value;
}

std::shared_ptr<std::string> SimpleConfigProcessor::get(std::string const& name) const {
	std::shared_ptr<std::string> value = _props.get(name);
	if ((!value) && _vars)
		value = _vars->get(name);
	return value;
}

bool SimpleConfigProcessor::containsKey(std::string const& name) const {
	if (_props.containsKey(name))
		return true;
	if (_vars && _vars->containsKey(name))
		return true;
	return false;
}

Config::Config(const std::string& confFileName, const std::string& appName)
:_confFileName(confFileName)
,_appName(appName)
,_cfgProc(*this)
,_simpleCfgProc(*this) {}

std::string searchConfigFile(List<std::string> const& configDirs, const std::string& configFile) {
	ConstIterator<std::shared_ptr<std::string> > i = configDirs.constIterator();
	while (i.hasNext()) {
		std::string const& dir = *i.next();
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

std::string Config::locateConfigFile(const std::string& fileName) const {
	ArrayList<std::string> confList;
	confList.add("/etc");
	confList.add(FileUtils::buildPath(_rootDir, "conf"));
	onBeforeSearch(confList);

	return searchConfigFile(confList, fileName);
}

/** @throws InitException */
void Config::internalInit(bool minimal) {
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

	std::string configDir = locateConfigFile(_confFileName);
	if (configDir.length() < 1)
		throw InitException(_HERE_, fmt::format("Could not locate config file '{}'", _confFileName).c_str());

	_confDir = configDir;

	clear();
	openConfigFile(minimal);
	//_vars.forEach(&dmp, nullptr);
	readConfig();
}

void Config::readConfig() {
	// read home dir
	_homeDir = getString("home", _rootDir);
	
	// read logdir
	_logDir = getString("logdir", "");
	if (_logDir.empty())
		_logDir = FileUtils::buildPath(_homeDir, "log");
}

void Config::openConfigFile(bool minimal) {
	std::string configFile = FileUtils::buildPath(_confDir, _confFileName);
	try {
		FileInputStream propsFile(configFile);
		if (minimal)
			load(propsFile, &_simpleCfgProc);
		else
			load(propsFile, &_cfgProc);
	} catch (IOException const& e) {
		throw InitException(_HERE_, fmt::format("I/O error reading config file: {}", e.getMessage()).c_str());
	}
}
	
void Config::shutdown() {
	Log::shutdown();
}

void Config::registerPropertySource(std::string const& name, std::shared_ptr<PropertySource> const& src) {
	_cfgProc.registerSource(name, src);
}

void Config::registerPropertySink(std::string const& name, ConfigProcessor::PropertySink const& sink) {
	_cfgProc.registerSink(name, sink);
}

} // namespace
