/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_PROPERTYSOURCE_H
#define H_SLIB_UTIL_PROPERTYSOURCE_H

#include "slib/exception/ValueException.h"

#include <unordered_map>
#include <string>

namespace slib {

class PropertySource {
protected:
	typedef std::string (PropertySource::*GetProperty)();
private:
	typedef std::unordered_map<std::string, GetProperty> PropertyMap;
	typedef PropertyMap::const_iterator PropMapConstIter;
	PropertyMap _properties;
	bool _initialized;
protected:
	void provideProperty(std::string const& name, GetProperty prop) {
		_properties[name] = prop;
	}
public:
	PropertySource()
	:_initialized(false) {
	}

	virtual ~PropertySource();

	std::string getProperty(const std::string& name);

	virtual void initialize() = 0;

	void init() {
		initialize();
		_initialized = true;
	}
};

} // namespace

#endif // H_SLIB_UTIL_PROPERTYSOURCE_H
