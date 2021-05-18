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

class PropertySource : public expr::LazyResolver {
protected:
	typedef SPtr<Object> (PropertySource::*GetProperty)() const;
private:
	typedef std::unordered_map<String, GetProperty> PropertyMap;
	typedef PropertyMap::const_iterator PropMapConstIter;
	PropertyMap _properties;
protected:
	void provideProperty(std::string const& name, GetProperty prop) {
		_properties[name] = prop;
	}
public:
	virtual ~PropertySource() override;

	virtual SPtr<Object> provideVar(String const& name) const override;
};

} // namespace

#endif // H_SLIB_UTIL_PROPERTYSOURCE_H
