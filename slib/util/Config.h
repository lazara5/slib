/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_UTIL_CONFIG_H__
#define __SLIB_UTIL_CONFIG_H__

#include "slib/util/Log.h"
#include "slib/util/StringUtils.h"
#include "slib/Numeric.h"
#include "slib/List.h"

#include "fmt/format.h"

#include <string>

namespace slib {

/** Manages a configuration file */
class Config {
protected:
	std::string _rootDir;
	std::string _homeDir;
	std::string _confFileName;
	std::string _appName;
	std::string _confDir;
	std::string _logDir;

	ValueMap _vars;
protected:
	/** @throws InitException */
	virtual void openConfigFile();

	/**
	 * @throws MissingValueException,
	 * @throws InitException
	 */
	std::string interpolate(std::string const& src) {
		return StringUtils::interpolate(src, _vars);
	}

	/** @throws NumberFormatException */
	template<class T, T MIN, T MAX>
	static const T& rangeCheck(const char *where, const T& value) {
		if ((value < MIN) || (value > MAX))
			throw NumberFormatException(where, "Value out of range");
		return value;
	}

private:
	virtual void readConfig();
public:
	Config(const std::string& confFileName, const std::string& appName);
	virtual ~Config() {}

	/** @throws InitException */
	virtual void init();

	virtual void onBeforeSearch(List<std::string> &pathList) {}

	static void shutdown();
	void forEach(bool (*callback)(void*, const std::string&, const std::string&), void *userData) const {
		_vars.forEach(callback, userData);
	}
public:
	/** @throws MissingValueException */
	std::string getValue(const std::string& name) const {
		return _vars.getVar(name);
	}

	std::string getValue(const std::string& name, const std::string& defaultValue) const {
		try {
			return getValue(name);
		} catch (MissingValueException const& e) {
			return defaultValue;
		}
	}

	/** @throws MissingValueException
	  * @throws NumberFormatException
	  */
	template <uint32_t MIN = slib::Integer::MIN_VALUE, uint32_t MAX = slib::Integer::MAX_VALUE>
	int getIntValue(const std::string& name) const {
		try {
			return rangeCheck<uint32_t, MIN, MAX>(_HERE_, Integer::parseInt(getValue(name)));
		} catch (NumberFormatException const& e) {
			throw NumberFormatException(_HERE_, fmt::format("Invalid integer value: {} ({})", name, e.getMessage()).c_str());
		}
	}

	/** @throws NumberFormatException */
	template <uint32_t MIN = slib::Integer::MIN_VALUE, uint32_t MAX = slib::Integer::MAX_VALUE>
	int getIntValue(const std::string& name, int defaultValue) const {
		try {
			return getIntValue<MIN, MAX>(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	/** @throws MissingValueException */
	bool getBoolValue(const std::string& name) const {
		return Boolean::parseBoolean(getValue(name));
	}

	bool getBoolValue(const std::string& name, bool defaultValue) const {
		try {
			return getBoolValue(name);
		} catch (MissingValueException const& e) {
			return defaultValue;
		}
	}

public:
	const std::string& getAppName() const { return _appName; }
	const std::string& getHomeDir() const { return _homeDir; }
	const std::string& getLogDir() const { return _logDir; }
	const std::string& getConfDir() const { return _confDir; }
};

} // namespace

#endif
