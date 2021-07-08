/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/StringView.h"

namespace slib {

const char *cStr(const char *str) {
	return str;
}

const char *strData(const char *str) {
	return str;
}

size_t strLen(const char *str) {
	return str ? strlen(str) : 0;
}

/*void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, StringView const& s) {
	f.writer().write("{}", s.c_str());
}*/

} // namespace slib
