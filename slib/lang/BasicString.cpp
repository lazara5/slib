/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/BasicString.h"
#include "slib/lang/String.h"

namespace slib {

int BasicString::compareTo(const BasicString &other) const {
	const char *buffer = c_str();
	const char *otherBuffer = other.c_str();

	if ((!buffer) || (!otherBuffer))
		throw NullPointerException(_HERE_);

	return strcmp(buffer, otherBuffer);
}

bool BasicString::equals(BasicString const& other) const {
	return equals(this, CPtr(other));
}

UPtr<String> BasicString::toUpperCase() const {
	const char *buffer = c_str();
	if (!buffer)
		return nullptr;
	size_t len = length();
	UPtr<String> res = newU<String>(buffer, length());
	char *resBuffer = res->str();
	for (size_t i = 0; i < len; i++)
		resBuffer[i] = ((char)toupper(resBuffer[i]));
	return res;
}

UPtr<String> BasicString::toLowerCase() const {
	const char *buffer = c_str();
	if (!buffer)
		return nullptr;
	size_t len = length();
	UPtr<String> res = newU<String>(buffer, length());
	char *resBuffer = res->str();
	for (size_t i = 0; i < len; i++)
		resBuffer[i] = ((char)tolower(resBuffer[i]));
	return res;
}

}
