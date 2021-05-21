/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Class.h"

#include "fmt/format.h"

namespace slib {

template<>
int32_t uintptrTHash<8>(size_t h) {
	uint32_t val = (uint32_t)(h ^ (h >> 32));
	return *(int32_t*)&val;
}

template<>
int32_t uintptrTHash<4>(size_t h) {
	return *(int32_t*)&h;
}

ClassCastException::ClassCastException(const char *where, const StringView &c1, const StringView &c2)
:Exception(where, "ClassCastException", fmt::format("Cannot cast from {} to {}", c1, c2).c_str()) {}

} // namespace slib
