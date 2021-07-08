/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/TemplateUtils.h"

namespace slib {

template<>
int32_t sizeTHash<8>(size_t h) {
	uint32_t val = (uint32_t)(h ^ (h >> 32));
	return *(int32_t*)&val;
}

template<>
int32_t sizeTHash<4>(size_t h) {
	return *(int32_t*)&h;
}

} // namespace slib

