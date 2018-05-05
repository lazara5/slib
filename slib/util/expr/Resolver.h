/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_RESOLVER_H
#define H_SLIB_UTIL_EXPR_RESOLVER_H

#include "slib/Object.h"
#include "slib/String.h"
#include "slib/collections/Map.h"

namespace slib {
namespace expr {

class Resolver : virtual public Object {
public:
	static Class const* _class;
public:
	virtual ~Resolver();
	/**
	 * Resolves a variable
	 *
	 * @param key  variable name
	 * @return variable value or nullptr if not defined
	 */
	virtual std::shared_ptr<Object> getVar(String const& key) = 0;
};

class MapResolver : public Resolver {
private:
	std::shared_ptr<Map<String, Object>> _map;
public:
	MapResolver(std::shared_ptr<Map<String, Object>> map)
	:_map(map) {}

	virtual ~MapResolver() override;

	virtual std::shared_ptr<Object> getVar(const String &key) override {
		return _map->get(key);
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_RESOLVER_H
