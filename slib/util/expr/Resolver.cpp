/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Resolver.h"

namespace slib {
namespace expr {

Resolver::~Resolver() {}

constexpr Class Resolver::_class;

MapResolver::~MapResolver() {}

} // namespace expr
} // namespace slib
