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
	std::shared_ptr<std::string> _hostname;

	/** IP addresses */
	std::shared_ptr<std::string> _ip;
	std::shared_ptr<std::string> _ipv4;
	std::shared_ptr<std::string> _ipv6;
public:
	SystemInfo();

	void initialize();

	/** get host name */
	std::shared_ptr<std::string> getHostname() const {
		return _hostname;
	}

	/** get first active non-loopback IP address */
	std::shared_ptr<std::string> getIp() const {
		return _ip;
	}

	/** get first active non-loopback IPv4 address */
	std::shared_ptr<std::string> getIpV4() const {
		return _ipv4;
	}

	/** get firstactive  non-loopback IPv6 address */
	std::shared_ptr<std::string> getIpV6() const {
		return _ipv6;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_SYSTEM_H
