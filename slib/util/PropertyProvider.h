/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_UTIL_PROPERTYPROVIDER_H__
#define __SLIB_UTIL_PROPERTYPROVIDER_H__

#include "slib/exception/ValueException.h"

#include <unordered_map>
#include <string>

namespace slib {

class PropertyProvider {
protected:
	typedef std::string (PropertyProvider::*Property)();
private:
	typedef std::unordered_map<std::string, Property> PropMap;
	typedef PropMap::const_iterator PropMapConstIter;
	PropMap _propRegistry;
	bool _initialized;
protected:
	void registerProperty(std::string const& name, Property prop) {
		_propRegistry[name] = prop;
	}
public:
	PropertyProvider()
	:_initialized(false) {
	}

	virtual ~PropertyProvider() {}

	std::string getProperty(const std::string& name) {
		if (!_initialized)
			init();
		PropMapConstIter i = _propRegistry.find(name);
		if (i == _propRegistry.end())
			throw MissingValueException(_HERE_, name.c_str());
		Property p = i->second;
		return (this->*p)();
	}

	virtual void initialize() = 0;

	void init() {
		initialize();
		_initialized = true;
	}
};

} // namespace

#endif
