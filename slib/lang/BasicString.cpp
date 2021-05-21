/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/BasicString.h"
#include "slib/lang/String.h"

namespace slib {

int BasicString::compareTo(const BasicString &other) const {
	const char *buffer = data();
	const char *otherBuffer = other.data();

	if ((!buffer) || (!otherBuffer))
		throw NullPointerException(_HERE_);

	size_t len = length();
	size_t otherLen = other.length();

	int common = std::min(len, otherLen);
	int res = memcmp(buffer, otherBuffer, common);
	if (res != 0)
		return res;
	return (len == otherLen) ? 0 : len - otherLen;
}

bool BasicString::equals(BasicString const& other) const {
	return equals(this, CPtr(other));
}

UPtr<String> BasicString::toUpperCase() const {
	const char *buffer = data();
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
	const char *buffer = data();
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
