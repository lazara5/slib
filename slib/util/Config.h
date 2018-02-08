/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_CONFIG_H
#define H_SLIB_UTIL_CONFIG_H

#include "slib/util/Log.h"
#include "slib/util/StringUtils.h"
#include "slib/Numeric.h"
#include "slib/collections/List.h"
#include "slib/collections/Properties.h"

#include "fmt/format.h"

#include <string>

namespace slib {

class ConfigProcessor : public Properties::LineProcessor, public ValueProvider<std::string, std::string> {
public:
	typedef std::function<void(std::string const&, std::string const&)> PropertySink;
private:
	typedef HashMap<std::string, std::string> VarMap;

	typedef std::unordered_map<std::string, std::shared_ptr<PropertySource>> SourceMap;
	typedef SourceMap::const_iterator SourceMapConstIter;

	typedef std::unordered_map<std::string, PropertySink> SinkMap;
	typedef SinkMap::const_iterator SinkMapConstIter;
private:
	Properties const& _props;
	std::unique_ptr<VarMap> _vars;
	std::unique_ptr<SourceMap> _sources;
	SinkMap _sinks;
public:
	ConfigProcessor(Properties const& props)
	:_props(props) {}

	virtual std::shared_ptr<std::string> processLine(const std::string& name,
													 const std::string& rawProperty) override;

	void registerSource(const std::string& name, std::shared_ptr<PropertySource> const& src) {
		if (!_sources)
			_sources = std::make_unique<SourceMap>();
		(*_sources)[name] = src;
	}

	void registerSink(const std::string& name, PropertySink const& s) {
		_sinks[name] = s;
	}

	bool sink(const std::string& sinkName, const std::string& name, const std::string& value);

	// ValueProvider interface
public:
	virtual std::shared_ptr<std::string> get(std::string  const& name) const override;
	virtual bool containsKey(std::string const& name) const override;
};

class SimpleConfigProcessor : public Properties::LineProcessor, public ValueProvider<std::string, std::string> {
private:
	typedef HashMap<std::string, std::string> VarMap;
private:
	Properties const& _props;
	std::unique_ptr<VarMap> _vars;
public:
	SimpleConfigProcessor(Properties const& props)
	:_props(props) {}

	virtual std::shared_ptr<std::string> processLine(const std::string& name,
													 const std::string& rawProperty) override;

	// ValueProvider interface
public:
	virtual std::shared_ptr<std::string> get(std::string  const& name) const override;
	virtual bool containsKey(std::string const& name) const override;
};

class Config : public Properties {
protected:
	std::string _rootDir;
	std::string _homeDir;
	std::string _confFileName;
	std::string _appName;
	std::string _confDir;
	std::string _logDir;

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
	Config(const std::string& confFileName, const std::string& appName);
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

	virtual std::string locateConfigFile(const std::string& fileName) const;

	virtual void onBeforeSearch(List<std::string> &pathList) const {}

	static void shutdown();
public:
	void registerPropertySource(const std::string& name, std::shared_ptr<PropertySource> const& src);
	void registerPropertySink(const std::string& name, ConfigProcessor::PropertySink const& sink);
public:
	const std::string& getAppName() const { return _appName; }
	const std::string& getHomeDir() const { return _homeDir; }
	const std::string& getLogDir() const { return _logDir; }
	const std::string& getConfDir() const { return _confDir; }
};

} // namespace slib

#endif // H_SLIB_UTIL_CONFIG_H
