/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/String.h"
#include "slib/util/Config.h"
#include "slib/util/Log.h"
#include "slib/util/FileUtils.h"
#include "slib/util/FilenameUtils.h"
#include "slib/collections/ArrayList.h"
#include "slib/io/FileInputStream.h"
#include "slib/util/expr/ExpressionEvaluator.h"

#include <iostream>
#include <fstream>
#include <string>

#define PROCSELFEXE "/proc/self/exe"

namespace slib {

using namespace expr;

const SPtr<Config::ConfigValue> ObjConfig::_nullValue = newS<ObjConfig::ConfigValue>();

const UPtr<Map<Class, ObjConfig::NewConfigValue>> ObjConfig::_mapper
	= newU<HashMap<Class, ObjConfig::NewConfigValue>, std::initializer_list<std::pair<SPtr<Class>, SPtr<ObjConfig::NewConfigValue>>>>({
	{
		newS<Class>(classOf<String>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newS<ConfigValue>(Class::cast<String>(obj));
		})
	},
	{
		newS<Class>(classOf<Long>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newS<ConfigValue>(Class::cast<Long>(obj)->longValue());
		})
	},
	{
		newS<Class>(classOf<Double>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newS<ConfigValue>(Class::cast<Double>(obj)->doubleValue());
		})
	},
	{
		newS<Class>(classOf<Boolean>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newS<ConfigValue>(Class::cast<Boolean>(obj)->booleanValue());
		})
	},
	{
		newS<Class>(classOf<Map<BasicString, Object>>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newS<ConfigValue>(Class::cast<Map<BasicString, Object>>(obj));
		})
	},
});

ConfigLoader::ConfigResolver::ConfigResolver() {
	char pathBuf[PATH_MAX] = "";
	struct stat s;

	if (stat(PROCSELFEXE, &s) < 0)
		perror("stat " PROCSELFEXE);
	else if (readlink(PROCSELFEXE, pathBuf, sizeof(pathBuf)) < 0) {
		perror("readlink " PROCSELFEXE);
		pathBuf[0] = '\0';
	}
}

SPtr<Object> ConfigLoader::ConfigResolver::getVar(String const& key, ValueDomain domain) const {
	if (StringView::equals(key, "_EXEDIR"_SV)) {
		return _exeDir;
	} else if (StringView::equals(key, "_CWD"_SV)) {
		return _cwd;
	}

	return nullptr;
}

ConfigLoader::ConfigLoader(SPtr<String> const& confFileName, SPtr<String> const& appName)
: _confFileName(confFileName)
, _appName(appName)
, _vars(newS<HashMap<String, Object>>())
, _quickResolver(expr::ChainedResolver::newInstance())
, _resolver(expr::ChainedResolver::newInstance()) {
	(*_quickResolver).add(newS<ConfigResolver>())
	.add("env"_SPTR, newS<EnvResolver>());

	(*_resolver)
		.add(_quickResolver)
		.add(_vars, ValueDomain::LOCAL);

	_searchPaths.add(newS<StringPair>("/etc"_SPTR, nullptr));
	_searchPaths.add(newS<StringPair>("${_EXEDIR}/conf"_SPTR, "${_EXEDIR}"_SPTR));
	_searchPaths.add(newS<StringPair>("${_CWD}/conf"_SPTR, "${_CWD}"_SPTR));
}

ConfigLoader& ConfigLoader::clearPaths() {
	_searchPaths.clear();
	return *this;
}

ConfigLoader& ConfigLoader::search(SPtr<String> const& path, SPtr<String> const& rootDir /* = nullptr */) {
	_searchPaths.add(newS<StringPair>(path, rootDir));
	return *this;
}

ConfigLoader& ConfigLoader::withResolver(SPtr<expr::Resolver> const& resolver) {
	_resolver->add(resolver);
	return *this;
}

ConfigLoader& ConfigLoader::withResolver(SPtr<String> const& name, SPtr<expr::Resolver> const& resolver) {
	_resolver->add(name, resolver);
	return *this;
}

/** @throws EvaluationException */
SPtr<ConfigLoader::StringPair> ConfigLoader::searchConfigDir() {
	UPtr<ConstIterator<SPtr<StringPair>>> i = _searchPaths.constIterator();
	while (i->hasNext()) {
		SPtr<StringPair> const& cfgEntry = i->next();
		UPtr<String> dir = ExpressionEvaluator::interpolate(*cfgEntry->_first, _quickResolver, true);
		if (dir) {
			UPtr<String> fileName = FilenameUtils::concat(dir, _confFileName);
			if (access(fileName->c_str(), 0) == 0) {
				UPtr<String> rootDir = (cfgEntry->_second)
					? ExpressionEvaluator::interpolate(*cfgEntry->_first, _quickResolver, true)
					: nullptr;
				return newS<StringPair>(std::move(dir), std::move(rootDir));
			}
		}
	}

	return nullptr;
}

SPtr<Config> ConfigLoader::load(bool quick) {
	try {
		SPtr<StringPair> conf = searchConfigDir();
		if (!conf)
			throw InitException(_HERE_, fmt::format("Could not locate config file '{}'", *_confFileName).c_str());

		SPtr<String> confDir(std::move(conf->_first));
		SPtr<String> appDir(std::move(conf->_second));

		UPtr<String> fileName = FilenameUtils::concat(confDir, _confFileName);

		UPtr<Array<uint8_t>> contents = FileUtils::readAllBytes(fileName);
		StringBuilder configContents("{");
		configContents.add(*contents)
			.add('}');
		SPtr<String> configText = configContents.toString();

		_vars->clear();
		SPtr<Object> parsedConfig = ExpressionEvaluator::expressionValue(configText,
			quick ? _quickResolver : _resolver);
		if (!parsedConfig)
			throw InitException(_HERE_, "Error parsing config");
		if (!instanceof<Map<BasicString, Object>>(*parsedConfig))
			throw InitException(_HERE_, fmt::format("Invalid config: expected Map<String, Object>, got {}", parsedConfig->getClass().getName()).c_str());
		SPtr<Map<BasicString, Object>> configRoot = Class::cast<Map<BasicString, Object>>(parsedConfig);

		UPtr<String> s = configRoot->toString();
		fmt::print("Config: {}\n", *s);

		return newS<Config>(configRoot, _appName, confDir, appDir);
	} catch (EvaluationException const& e) {
		throw InitException(_HERE_, e);
	} catch (IOException const& e) {
		throw InitException(_HERE_, e);
	}
}

/** @throws InitException */
/*void Config::internalInit(bool minimal) {
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

void Config::openConfigFile(bool minimal) {
	UPtr<String> configFile = FileUtils::buildPath(CPtr(_confDir), CPtr(_confFileName));
	try {
		FileInputStream propsFile(CPtr(configFile));
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
}*/

} // namespace
