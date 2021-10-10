/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/BasicString.h"
#include "slib/lang/String.h"

namespace slib {

int IString::compareTo(const IString &other) const {
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

bool IString::equals(IString const& other) const {
	return StringView::equals(this, other);
}

UPtr<String> IString::toUpperCase() const {
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

UPtr<String> IString::toLowerCase() const {
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
