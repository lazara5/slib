/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/String.h"
#include "slib/util/Config.h"
#include "slib/util/Log.h"
#include "slib/util/FileUtils.h"
#include "slib/collections/ArrayList.h"
#include "slib/io/FileInputStream.h"
#include "slib/util/expr/ExpressionEvaluator.h"

#include <iostream>
#include <fstream>
#include <string>

#define PROCSELFEXE "/proc/self/exe"

namespace slib {

using namespace expr;

UPtr<String> ConfigProcessor::processLine(String const& name, SPtr<String> const& rawProperty) {
	//SPtr<String> value = StringUtils::interpolate(rawProperty, *this, false);
	SPtr<Object> value = ExpressionEvaluator::smartInterpolate(*rawProperty, *this, false);

	if (String::startsWith(CPtr(name), '@')) {
		if (!_vars)
			_vars = std::make_unique<HashMap<String, Object>>();
		_vars->put(String::substring(CPtr(name), 1), value);
		return nullptr;
	} else if (String::endsWith(CPtr(name), ']')) {
		ptrdiff_t openBracket = String::lastIndexOf(CPtr(name), '[');
		if (openBracket > 0) {
			std::string mapName = String::trim(CPtr(String::substring(CPtr(name), 0, (size_t)openBracket)));
			std::string mapEntry = String::trim(CPtr(String::substring(CPtr(name), (size_t)openBracket + 1, name.length() - 1)));
			if ((!mapName.empty()) && (!mapEntry.empty())) {
				bool sunk = sink(mapName, mapEntry, value);
				if (sunk)
					return nullptr;
			}
		}
	}
	return Value::asString(value);
}

bool ConfigProcessor::sink(String const& sinkName, String const& name, SPtr<Object> const& value) {
	SinkMapConstIter sink = _sinks.find(sinkName);
	if (sink == _sinks.end())
		return false;
	sink->second(name, value);
	return true;
}

SPtr<Object> ConfigProcessor::getVar(String const& name) const {
	SPtr<Object> value = _props.get(name);
	if ((!value) && _vars)
		value = _vars->get(name);
	if ((!value) && _sources) {
		std::ptrdiff_t dotPos;
		if ((dotPos = String::lastIndexOf(CPtr(name), '.')) > 0) {
			std::string providerName = String::substring(CPtr(name), 0, (size_t)dotPos);
			std::string propertyName = String::substring(CPtr(name), (size_t)dotPos + 1);
			SourceMapConstIter provider = _sources->find(providerName);
			if (provider != _sources->end())
				return provider->second->getVar(propertyName);
		}
	}
	return value;
}

/*bool ConfigProcessor::containsKey(std::string const& name) const {
	if (_props.containsKey(name))
		return true;
	if (_vars && _vars->containsKey(name))
		return true;
	if (_sources) {
		std::ptrdiff_t dotPos;
		if ((dotPos = String::lastIndexOf(CPtr(name), '.')) > 0) {
			std::string providerName = String::substring(CPtr(name), 0, (size_t)dotPos);
			std::string propertyName = String::substring(CPtr(name), (size_t)dotPos + 1);
			SourceMapConstIter provider = _sources->find(providerName);
			if ((provider != _sources->end()) && (provider->second->containsKey(propertyName)))
				return true;
		}
	}
	return false;
}*/

UPtr<String> SimpleConfigProcessor::processLine(String const& name, SPtr<String> const& rawProperty) {
	//SPtr<String> value = StringUtils::interpolate(rawProperty, *this, true);
	SPtr<Object> value = ExpressionEvaluator::smartInterpolate(*rawProperty, *this, true);
	if (String::startsWith(CPtr(name), '@')) {
		if (!_vars)
			_vars = std::make_unique<HashMap<String, Object>>();
		_vars->put(String::substring(CPtr(name), 1), value);
		return nullptr;
	}
	return Value::asString(value);
}

SPtr<Object> SimpleConfigProcessor::getVar(String const& name) const {
	SPtr<Object> value = _props.get(name);
	if ((!value) && _vars)
		value = _vars->get(name);
	return value;
}

/*bool SimpleConfigProcessor::containsKey(std::string const& name) const {
	if (_props.containsKey(name))
		return true;
	if (_vars && _vars->containsKey(name))
		return true;
	return false;
}*/

Config::Config(String const& confFileName, String const& appName)
:_confFileName(confFileName)
,_appName(appName)
,_cfgProc(*this)
,_simpleCfgProc(*this) {}

SPtr<String> searchConfigFile(List<String> const& configDirs, String const& configFile) {
	ConstIterator<SPtr<String>> i = configDirs.constIterator();
	while (i.hasNext()) {
		SPtr<String> const& dir = i.next();
		UPtr<String> fileName = FileUtils::buildPath(*dir, configFile);
		if (!access(fileName->c_str(), 0))
			return dir;
	}

	return nullptr;
}

bool dmp(void *data, const std::string& k, const std::string& v) {
	printf("%s = %s\n", k.c_str(), v.c_str());
	return true;
}

SPtr<String> Config::locateConfigFile(String const& fileName) const {
	ArrayList<String> confList;
	confList.emplace<String>("/etc");
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
		if (getcwd(pathBuf, sizeof(pathBuf)) == nullptr)
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

	SPtr<String> configDir = locateConfigFile(_confFileName);
	if (!configDir)
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
	if (!_logDir)
		_logDir = FileUtils::buildPath(*_homeDir, "log");
}

void Config::openConfigFile(bool minimal) {
	UPtr<String> configFile = FileUtils::buildPath(*_confDir, _confFileName);
	try {
		FileInputStream propsFile(*configFile);
		if (minimal)
			load(propsFile, &_simpleCfgProc);
		else
			load(propsFile, &_cfgProc);
	} catch (IOException const& e) {
		throw InitException(_HERE_, fmt::format("I/O error reading config file '{}': {}", *configFile, e.getMessage()).c_str());
	}
}
	
void Config::shutdown() {
	Log::shutdown();
}

void Config::registerPropertySource(String const& name, SPtr<PropertySource> const& src) {
	_cfgProc.registerSource(name, src);
}

void Config::registerPropertySink(String const& name, ConfigProcessor::PropertySink const& sink) {
	_cfgProc.registerSink(name, sink);
}

} // namespace
