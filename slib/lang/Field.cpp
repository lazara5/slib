/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Field.h"

namespace slib {

namespace internal {

using namespace nonstd;

int64_t _fieldGetLong(FieldValue const& val) {
	if ((val._type & FieldValType::NUMBER) != 0) {
		switch (val._type) {
			case FieldValType::INT64:
				return get<int64_t>(val._val);
			case FieldValType::DOUBLE: {
				double d = get<double>(val._val);
				if (Number::isMathematicalInteger(d)) {
					if ((d <= Double::MAX_SAFE_INTEGER) && (d >= Double::MIN_SAFE_INTEGER))
						return (int64_t)d;
				}
			}
			default:
				break;
		}
	}
	THROW(IllegalArgumentException);
}

} // namespace internal

} // namespace slib
