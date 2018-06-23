/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_RESOLVER_H
#define H_SLIB_UTIL_EXPR_RESOLVER_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"
#include "slib/collections/Map.h"

namespace slib {
namespace expr {

class Resolver : virtual public Object {
public:
	virtual ~Resolver() override;

	static constexpr Class _class = RESOLVERCLASS;

	virtual Class const& getClass() const override {
		return RESOLVERCLASS;
	}

	/**
	 * Resolves a variable
	 *
	 * @param key  variable name
	 * @return variable value or nullptr if not defined
	 */
	virtual SPtr<Object> getVar(String const& key) const = 0;
};

class MapResolver : public Resolver {
private:
	SPtr<Map<String, Object>> _map;
public:
	MapResolver(SPtr<Map<String, Object>> map)
	:_map(map) {}

	virtual ~MapResolver() override;

	virtual SPtr<Object> getVar(const String &key) const override {
		return _map->get(key);
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_RESOLVER_H
