/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _SLIB_UTIL_STRINGUTILS_H_
#define _SLIB_UTIL_STRINGUTILS_H_

#include "slib/exception/Exception.h"
#include "slib/util/PropertyProvider.h"
#include "slib/StringBuilder.h"

#include "fmt/format.h"

#include <string>
#include <unordered_map>

namespace slib {

/** map of std::string keys and values; allows retrieving properties from external PropertyProvider sources. */
class ValueMap {
private:
	typedef std::unordered_map<std::string, std::string> StringMap;
	typedef StringMap::const_iterator StringMapConstIter;

	typedef std::unordered_map<std::string, PropertyProvider*> ProviderMap;
	typedef ProviderMap::const_iterator ProviderMapConstIter;

	typedef std::function<void(std::string const&, std::string const&)> ConfigSink;
	typedef std::unordered_map<std::string, ConfigSink> SinkMap;
	typedef SinkMap::const_iterator SinkMapConstIter;
private:
	StringMap _vars;
	ProviderMap _providers;
	SinkMap _sinks;
public:
	void set(const std::string& name, const std::string& value) {
		_vars[name] = value;
	}

	/** @throws MissingValueException */
	std::string getVar(const std::string& name) const;
	
	/** @throws MissingValueException, InitException */
	std::string get(const std::string& name);

	bool sink(const std::string& sinkName, const std::string& name, const std::string& value);
	
	void registerProvider(const std::string& name, PropertyProvider *p) {
		_providers[name] = p;
	}

	void registerSink(const std::string& name, ConfigSink s) {
		_sinks[name] = s;
	}

	void forEach(bool (*callback)(void*, const std::string&, const std::string&), void *userData) const {
		for (StringMapConstIter i = _vars.begin(); i != _vars.end(); i++) {
			bool cont = callback(userData, i->first, i->second);
			if (!cont)
				return;
		}
	}
};

/** Convenience class for building XML-escaped strings */
class XMLString : public StringBuilder {
public:
	XMLString(const char *str);

	XMLString(const StringBuilder& s)
	:XMLString(s.c_str()) {}

	XMLString(const std::string& s)
	:XMLString(s.c_str()) {}
};

class StringUtils {
public:
	static std::string formatException(std::string const& msg, Exception const& e) {
		return fmt::format("{} [{}: {} ({})]", msg, e.getName(), e.getMessage(), e.where());
	}

	static std::string formatErrno(int err) {
		char buffer[1024];
		buffer[0] = 0;

		const char *msg = strerror_r(err, buffer, sizeof(buffer));
		return fmt::format("{:d} ({})", err, msg);
	}

	static std::string formatErrno() {
		return formatErrno(errno);
	}

	static std::string interpolate(std::string const& src, ValueMap& vars);
};

} // namespace

#endif
