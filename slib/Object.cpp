/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/Object.h"
#include "slib/String.h"

#include "fmt/format.h"

namespace slib {

Class const* Object::_class = OBJECTCLASS();

String Object::toString() const {
	return String(fmt::format("{}@{x}", getClass()->getName(), hashCode()));
}

} // namespace slib
