/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/Numeric.h"

namespace slib {

Number::~Number() {}

Class const* Number::_class = NUMBERCLASS();

Class const* Integer::_class = INTEGERCLASS();

double Integer::doubleValue() const {
	return (double)_value;
}

Class const* UInt::_class = UINTCLASS();

double UInt::doubleValue() const {
	return (double)_value;
}

Class const* Long::_class = LONGCLASS();

double Long::doubleValue() const {
	return (double)_value;
}

Class const* ULong::_class = ULONGCLASS();

double ULong::doubleValue() const {
	return (double)_value;
}

Class const* Double::_class = DOUBLECLASS();

double Double::doubleValue() const {
	return _value;
}

Class const* Boolean::_class = BOOLEANCLASS();

Boolean::~Boolean() {}

} // namespace slib
