/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_STRINGUTILS_H
#define H_SLIB_UTIL_STRINGUTILS_H

#include "slib/exception/Exception.h"
#include "slib/util/PropertySource.h"
#include "slib/collections/Map.h"
#include "slib/StringBuilder.h"


#include "fmt/format.h"

#include <string>
#include <unordered_map>
#include <functional>

namespace slib {

/** Convenience class for building XML-escaped strings */
class XMLString : public StringBuilder {
public:
	XMLString(const char *str);

	XMLString(const StringBuilder& s)
	:XMLString(s.c_str()) {}

	XMLString(const std::string& s)
	:XMLString(s.c_str()) {}
};

class StringUtils {
public:
	static std::string formatException(std::string const& msg, Exception const& e) {
		return fmt::format("{} [{}: {} ({})]", msg, e.getName(), e.getMessage(), e.where());
	}

	static std::string formatErrno(int err) {
		char buffer[1024];
		buffer[0] = 0;

		const char *msg = strerror_r(err, buffer, sizeof(buffer));
		return fmt::format("{:d} ({})", err, msg);
	}

	static std::string formatErrno() {
		return formatErrno(errno);
	}

	static std::shared_ptr<std::string> interpolate(std::string const& src,
													ValueProvider<std::string, std::string> const& vars, bool ignoreUndefined);
};

} // namespace slib

#endif // H_SLIB_UTIL_STRINGUTILS_H
