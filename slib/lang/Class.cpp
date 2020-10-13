/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Class.h"

#include "fmt/format.h"

namespace slib {

ClassCastException::ClassCastException(const char *where, const char *c1, const char *c2)
:Exception(where, "ClassCastException", fmt::format("Cannot cast from {} to {}", c1, c2).c_str()) {}

} // namespace slib
