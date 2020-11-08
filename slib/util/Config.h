/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_CONFIG_H
#define H_SLIB_UTIL_CONFIG_H

#include "slib/lang/String.h"
//#include "slib/util/Log.h"
#include "slib/util/StringUtils.h"
#include "slib/lang/Numeric.h"
#include "slib/collections/List.h"
#include "slib/collections/Properties.h"
#include "slib/util/expr/Resolver.h"

#include "fmt/format.h"

#include <string>

namespace slib {

class ConfigProcessor : public Properties::LineProcessor, /*public ValueProvider<std::string, std::string>,*/ public expr::Resolver {
public:
	typedef std::function<void(String const&, SPtr<Object> const&)> PropertySink;
private:
	typedef Map<String, Object> VarMap;

	typedef std::unordered_map<String, SPtr<PropertySource>> SourceMap;
	typedef SourceMap::const_iterator SourceMapConstIter;

	typedef std::unordered_map<String, PropertySink> SinkMap;
	typedef SinkMap::const_iterator SinkMapConstIter;
private:
	Properties const& _props;
	UPtr<VarMap> _vars;
	UPtr<SourceMap> _sources;
	SinkMap _sinks;
public:
	ConfigProcessor(Properties const& props)
	:_props(props) {}

	virtual ~ConfigProcessor() override {}

	virtual UPtr<String> processLine(SPtr<String> const& name, SPtr<String> const& rawProperty) override;

	void registerSource(String const& name, SPtr<PropertySource> const& src) {
		if (!_sources)
			_sources = std::make_unique<SourceMap>();
		(*_sources)[name] = src;
	}

	void registerSink(String const& name, PropertySink const& s) {
		_sinks[name] = s;
	}

	bool sink(String const& sinkName, String const& name, SPtr<Object> const& value);

	// ValueProvider interface
	// Resolver interface
public:
	//virtual SPtr<std::string> get(std::string const& name) const override;
	//virtual bool containsKey(std::string const& name) const override;
	virtual SPtr<Object> getVar(String const& key) const override;
};

class SimpleConfigProcessor : public Properties::LineProcessor, /*public ValueProvider<std::string, std::string>*/ public expr::Resolver {
private:
	typedef Map<String, Object> VarMap;
private:
	Properties const& _props;
	UPtr<VarMap> _vars;
public:
	SimpleConfigProcessor(Properties const& props)
	:_props(props) {}

	virtual UPtr<String> processLine(SPtr<String> const& name, SPtr<String> const& rawProperty) override;

	// ValueProvider interface
	// Resolver interface
public:
	//virtual SPtr<String> get(String  const& name) const override;
	//virtual bool containsKey(std::string const& name) const override;
	virtual SPtr<Object> getVar(String const& key) const override;
};

class Config : public Properties {
protected:
	std::string _rootDir;
	SPtr<String> _homeDir;
	String _confFileName;
	String _appName;
	SPtr<String> _confDir;
	SPtr<String> _logDir;

	ConfigProcessor _cfgProc;
	SimpleConfigProcessor _simpleCfgProc;
protected:
	/** @throws InitException */
	void openConfigFile(bool minimal);

	/** @throws NumberFormatException */
	template<class T, T MIN, T MAX>
	static const T& rangeCheck(const char *where, const T& value) {
		if ((value < MIN) || (value > MAX))
			throw NumberFormatException(where, "Value out of range");
		return value;
	}

private:
	virtual void readConfig();

	/**
	 * @param minimal  perform a minimal load (no sources, no sinks)
	 * @throws InitException
	 */
	void internalInit(bool minimal);
public:
	Config(String const& confFileName, String const& appName);
	virtual ~Config() {}

	/** @throws InitException */
	virtual void init() {
		internalInit(false);
	}

	/**
	 * Just enough initialization to locate the pid file.
	 * Does not run sources or sinks.
	 * Does not throw errors on undefined variables!
	 * @throws InitException
	 */
	virtual void initMinimal() {
		internalInit(true);
	}

	virtual SPtr<String> locateConfigFile(String const& fileName) const;

	virtual void onBeforeSearch(List<String> &/*pathList*/) const {}

	static void shutdown();
public:
	void registerPropertySource(String const& name, SPtr<PropertySource> const& src);
	void registerPropertySink(String const& name, ConfigProcessor::PropertySink const& sink);
public:
	String const& getAppName() const { return _appName; }
	SPtr<String> getHomeDir() const { return _homeDir; }
	SPtr<String> getLogDir() const { return _logDir; }
	SPtr<String> getConfDir() const { return _confDir; }
};

} // namespace slib

#endif // H_SLIB_UTIL_CONFIG_H
