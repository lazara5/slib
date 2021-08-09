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
	SPtr<String> _hostname;

	/** IP addresses */
	SPtr<String> _ip;
	SPtr<String> _ipv4;
	SPtr<String> _ipv6;
public:
	SystemInfo();

	virtual void initialize() override;

	/** get host name */
	SPtr<Object> getHostname() const {
		return _hostname;
	}

	/** get first active non-loopback IP address */
	SPtr<Object> getIp() const {
		return _ip;
	}

	/** get first active non-loopback IPv4 address */
	SPtr<Object> getIpV4() const {
		return _ipv4;
	}

	/** get firstactive  non-loopback IPv6 address */
	SPtr<Object> getIpV6() const {
		return _ipv6;
	}
};

class EnvResolver : public expr::Resolver {
public:
	virtual SPtr<Object> getVar(String const& key, expr::ValueDomain valueDomain) const override {
		if (valueDomain != expr::ValueDomain::DEFAULT)
			return nullptr;
		char *var = getenv(key.c_str());
		return var ? newS<String>(var) : nullptr;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_SYSTEM_H
