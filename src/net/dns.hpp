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
class DNSResponse {

    std::string name;
    std::string query_type;
    std::string query_class;
    std::string resolver;
    int code;
    double rtt;
    int ttl;

    std::vector<std::string> results;

public:
    DNSResponse(void);

    DNSResponse(std::string name, std::string query_type,
                std::string query_class, std::string resolver,
                int code, char type, int count, int ttl, double rtt,
                void *addresses, IghtLibevent *libevent = NULL,
                int start_from=0);

    std::vector<std::string> get_results(void) {
        return results;
    }
    std::string get_query_name(void) {
        return name;
    }
    std::string get_query_type(void) {
        return query_type;
    }
    std::string get_query_class(void) {
        return query_class;
    }
    std::string get_reply_authoritative(void) {
        return "unknown";  /* TODO */
    }
    std::vector<std::string> get_resolver(void) {
        if (resolver == "") {
            return {"<default>", "53"};
        }
        auto pos = resolver.find(":");
        if (pos == std::string::npos) {
            return {resolver, "53"};
        }
        return {resolver.substr(0, pos),
            resolver.substr(pos + 1)};
    }
    int get_evdns_status(void) {
        return code;
    }
    std::string get_failure(void) {
        return map_failure_(code);
    }
    int get_ttl(void) {
        return ttl;
    }
    double get_rtt(void) {
        return rtt;
    }

    static std::string map_failure_(int code);  // Public to ease testing
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
               evdns_base *dnsb = NULL, std::string resolver = "",
               IghtLibevent *libevent = NULL);

    DNSRequest(DNSRequest& /*other*/) = delete;
    DNSRequest& operator=(DNSRequest& /*other*/) = delete;

    DNSRequest(DNSRequest&& /*other*/) = default;
    DNSRequest& operator=(DNSRequest&& /*other*/) = default;

    void cancel(void);

    ~DNSRequest(void) {
        cancel();
    }
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
    std::string nameserver = "";
    evdns_base *base = NULL;
    IghtLibevent *libevent = IghtGlobalLibevent::get();

    void cleanup(void);

  public:
    DNSResolver(std::string nameserver = "", unsigned attempts = 0,
                double timeout = 0.0,
                IghtPoller *poller = NULL, IghtLibevent *lev = NULL);

    evdns_base *get_evdns_base(void);

    // Syntactic sugar:
    DNSRequest request(std::string query, std::string address,
                       std::function<void(DNSResponse&&)>&& func) {
        return DNSRequest(query, address, std::move(func), get_evdns_base(),
                          nameserver, libevent);
    }

    ~DNSResolver(void) {
        cleanup();
    }

    DNSResolver(DNSResolver& /*other*/) = delete;
    DNSResolver& operator=(DNSResolver& /*other*/) = delete;

    DNSResolver(DNSResolver&& /*other*/) = default;
    DNSResolver& operator=(DNSResolver&& /*other*/) = default;
};

}  // namespace
#endif  // LIBIGHT_NET_DNS_HPP
