/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_OBJECT_H
#define H_SLIB_OBJECT_H

#include "slib/Class.h"

#include <stdint.h>

namespace slib {

class String;

class Object {
public:
	virtual ~Object() {}

	virtual Class const& getClass() const {
		return objectClass;
	}

	virtual int32_t hashCode() const {
		uint64_t value = (uint64_t)this;
		return (int32_t)(value ^ (value >> 32));
	}

	virtual String toString() const;
};

} // namespace slib

#endif // H_SLIB_OBJECT_H
