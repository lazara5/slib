/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_SYSTEMINFO_H
#define H_SLIB_UTIL_SYSTEMINFO_H

#include "slib/util/PropertySource.h"

namespace slib {

class SystemInfo : public PropertySource {
private:
	/** hostname */
	std::string _hostname;

	/** IP addresses */
	std::string _ip;
	std::string _ipv4;
	std::string _ipv6;
public:
	SystemInfo();

	void initialize();

	/** get host name */
	std::string getHostname() {
		return _hostname;
	}

	/** get first active non-loopback IP address */
	std::string getIp() {
		return _ip;
	}

	/** get first active non-loopback IPv4 address */
	std::string getIpV4() {
		return _ipv4;
	}

	/** get firstactive  non-loopback IPv6 address */
	std::string getIpV6() {
		return _ipv6;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_SYSTEM_H
