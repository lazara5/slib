/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_PROPERTYSOURCE_H
#define H_SLIB_UTIL_PROPERTYSOURCE_H

#include "slib/collections/Map.h"
#include "slib/exception/ValueException.h"
#include "slib/util/expr/Resolver.h"

#include <unordered_map>
#include <string>

namespace slib {

class PropertySource : public expr::Resolver {
protected:
	typedef SPtr<Object> (PropertySource::*GetProperty)() const;
private:
	typedef std::unordered_map<String, GetProperty> PropertyMap;
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

	virtual ~PropertySource() override;

	virtual void initialize() = 0;

	void init() {
		initialize();
		_initialized = true;
	}

	virtual SPtr<Object> getVar(String const& name) const override;
};

} // namespace

#endif // H_SLIB_UTIL_PROPERTYSOURCE_H
