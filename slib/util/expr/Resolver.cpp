/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Resolver.h"

namespace slib {
namespace expr {

Resolver::~Resolver() {}

MapResolver::~MapResolver() {}

ChainedResolver::~ChainedResolver() {}

SPtr<Object> ChainedResolver::getVar(const String &key, ValueDomain domain) const {
	SPtr<Resolver> res = _namedResolvers->get(key);
	if (res)
		return res;
	auto i = _resolvers->constIterator();
	for (auto resolver : *i) {
		SPtr<Object> res = resolver->getVar(key, domain);
		if (res)
			return res;
	}
	return nullptr;
}

void ChainedResolver::setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) {
	uint8_t iDomain = static_cast<uint8_t>(domain);
	if (_writableResolver[iDomain])
		_writableResolver[iDomain]->setVar(key, value, domain);
	else
		Resolver::setVar(key, value, domain);
}

} // namespace expr
} // namespace slib
