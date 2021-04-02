/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Resolver.h"

namespace slib {
namespace expr {

Resolver::~Resolver() {}

MapResolver::~MapResolver() {}

ChainedResolver::~ChainedResolver() {}

SPtr<Object> ChainedResolver::getVar(const String &key) const {
	SPtr<Resolver> res = _namedResolvers->get(key);
	if (res)
		return res;
	for (auto resolver : _resolvers->constIterator()) {
		SPtr<Object> res = resolver->getVar(key);
		if (res)
			return res;
	}
	return nullptr;
}

void ChainedResolver::setVar(String const& key, SPtr<Object> const& value) {
	if (_writableResolver)
		_writableResolver->setVar(key, value);
	else
		Resolver::setVar(key, value);
}

} // namespace expr
} // namespace slib
