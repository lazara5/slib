#include "slib/util/SystemInfo.h"
#include "slib/util/Log.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>

extern slib::Log logger;

namespace slib {

SystemInfo::SystemInfo() {
	provideProperty("hostname",	static_cast<GetProperty>(&SystemInfo::getHostname));
	provideProperty("ip", 		static_cast<GetProperty>(&SystemInfo::getIp));
	provideProperty("ipv4", 	static_cast<GetProperty>(&SystemInfo::getIpV4));
	provideProperty("ipv6", 	static_cast<GetProperty>(&SystemInfo::getIpV6));
}

int getIPAddrs(std::string& ip, std::string& ipv4, std::string& ipv6) {
	struct ifaddrs *ifaddr, *ifa;
	int family, n;

	if (getifaddrs(&ifaddr) == -1) {
		logger.warnf("[SystemInfo] getifaddrs() failed, errno={}", StringUtils::formatErrno());
		return -1;
	}

	char ipstr[128];

	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL)
			continue;

		// ignore loopback, not running interfaces
		if ((strcmp("lo", ifa->ifa_name) == 0) || !(ifa->ifa_flags & (IFF_RUNNING)))
			continue;

		family = ifa->ifa_addr->sa_family;
		const void *addr;

		if (family == AF_INET || family == AF_INET6) {
			addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			const char *ipaddr = inet_ntop(family, addr, ipstr, sizeof(ipstr));

			if (ipaddr) {
				if (ip.empty())
					ip = ipaddr;
				if (family == AF_INET) {
					if (ipv4.empty())
						ipv4 = ipaddr;
				} else if (family == AF_INET6) {
					if (ipv6.empty())
						ipv6 = ipaddr;
				}
			}
		}
	}
	freeifaddrs(ifaddr);
	return 0;
}

void SystemInfo::initialize() {
	getIPAddrs(_ip, _ipv4, _ipv6);

	// hostname
	char hostname[256];

	if (gethostname(hostname, sizeof(hostname)) < 0) {
		logger.warnf("[SystemInfo] Failed to determine hostname, errno={}", StringUtils::formatErrno());
		_hostname = "localhost";
	} else
		_hostname = hostname;
}

} // namespace slib
