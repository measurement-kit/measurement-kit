/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_DNS_HPP
# define LIBIGHT_NET_DNS_HPP

//
// DNS client functionality
//

#include "common/poller.h"

#include <functional>
#include <vector>
#include <string>

struct evdns_base;  // Internally we use evdns

namespace ight {

class DNSRequestImpl;  // Defined in net/dns.cpp

//
// DNS response.
//
// The constructor receives the fields returned by evdns and converts
// them in a format suitable to compile OONI's reports.
//
struct DNSResponse {

    std::string name;
    std::string query_type;
    std::string resolver;
    int code;
    std::string reply_type;
    std::string reply_class;
    int ttl;
    double rtt;

    std::vector<std::string> results;

    DNSResponse(void);

    DNSResponse(std::string name, std::string query_type, std::string resolver,
                int code, char type, int count, int ttl, double rtt,
                void *addresses);
};

//
// Async DNS request.
//
// This is the toplevel class that you should use to issue async
// DNS requests; it supports A, AAAA and PTR queries.
//
// DNS requests issued using directly this class use the default DNS
// resolver of libight; use a DNSResolver object to issue DNS requests
// that are bound to a specific DNS resolver.
//
class DNSRequest {
    DNSRequestImpl *impl = nullptr;

  public:
    DNSRequest(std::string query, std::string address,
               std::function<void(DNSResponse&&)>&& func,
               evdns_base *dnsb = NULL,
               std::string resolver = "default");

    DNSRequest(DNSRequest& /*other*/) = delete;
    DNSRequest& operator=(DNSRequest& /*other*/) = delete;
    DNSRequest(DNSRequest&& other) {
        std::swap(impl, other.impl);
    }
    DNSRequest& operator=(DNSRequest&& other) {
        std::swap(impl, other.impl);
        return (*this);
    }

    ~DNSRequest(void);
};

//
// DNS Resolver object.
//
// This object can be used to construct specific DNS resolvers that
// differ from the default DNS resolver of libight.
//
// In other words, to use the default DNS resolver, one does not need
// to create an instance of this object.
//
// The default constructor creates a DNS resolver implementing the
// same options of the the default one (i.e., `/etc/resolv.conf` is
// parsed and requests are sent three times before timeout).
//
// Different settings can be selected by passing different options
// to the constructor method.
//
// Once the object is created, it allows to issue DNS requests bound
// to that DNS resolver rather than on the default one.
//
class DNSResolver {
    std::string nameserver = "default";
    evdns_base *base = NULL;

    void cleanup(void);

  public:
    DNSResolver(std::string nameserver = "", std::string attempts = "",
                IghtPoller *poller = NULL);

    evdns_base *get_evdns_base(void);

    // Syntactic sugar:
    DNSRequest request(std::string query, std::string address,
                       std::function<void(DNSResponse&&)>&& func) {
        return DNSRequest(query, address, std::move(func), get_evdns_base(),
                          nameserver);
    }

    ~DNSResolver(void) {
        cleanup();
    }

    DNSResolver(DNSResolver& /*other*/) = delete;
    DNSResolver& operator=(DNSResolver& /*other*/) = delete;
    DNSResolver(DNSResolver&& other) {
        std::swap(base, other.base);
    }
    DNSResolver& operator=(DNSResolver&& other) {
        std::swap(base, other.base);
        return *this;
    }
};

}  // namespace
#endif  // LIBIGHT_NET_DNS_HPP
