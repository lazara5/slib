/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/Numeric.h"

namespace slib {

Number::~Number() {}

double Integer::doubleValue() const {
	return (double)_value;
}

double UInt::doubleValue() const {
	return (double)_value;
}

double Long::doubleValue() const {
	return (double)_value;
}

double ULong::doubleValue() const {
	return (double)_value;
}

double Double::doubleValue() const {
	return _value;
}

} // namespace slib
