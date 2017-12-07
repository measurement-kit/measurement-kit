// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_DNS_SYSTEM_RESOLVER_HPP
#define PRIVATE_DNS_SYSTEM_RESOLVER_HPP

#include "private/common/mock.hpp"
#include <measurement_kit/dns.hpp>

#include "../dns/getaddrinfo_async.hpp"

namespace mk {
namespace dns {

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void system_resolver(QueryClass dns_class, QueryType dns_type, std::string name,
        Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error, SharedPtr<Message>> cb) {
    Query query;
    addrinfo hints = {};
    /*
     * Note: here we pass empty flags. It used to be AI_ALL | AI_V4MAPPED but
     * Android did not like it and, honestly, I do not think having back v4
     * mapped addresses would be super useful to us (or in general) given that
     * when we want to connect we usually connect trying to resolve AAAA and
     * A at the same time. Keeping the feature conditionally may lead to
     * odd behaviors in corner cases and I'd like to avoid future headaches.
     */
    hints.ai_socktype = SOCK_STREAM;

    if (dns_class != MK_DNS_CLASS_IN) {
        cb(UnsupportedClassError(), {});
        return;
    }

    if (dns_type == MK_DNS_TYPE_A) {
        hints.ai_family = AF_INET;
    } else if (dns_type == MK_DNS_TYPE_AAAA) {
        hints.ai_family = AF_INET6;
    } else if (dns_type == MK_DNS_TYPE_CNAME) {
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags |= AI_CANONNAME;
    } else {
        cb(UnsupportedTypeError(), {});
        return;
    }

    /*
     * When running OONI tests, we're interested to know not only the IPs
     * associated with a specific name, but also to the CNAME.
     */
    ErrorOr<bool> also_cname = settings.get("dns/resolve_also_cname", false);
    if (!also_cname) {
        cb(also_cname.as_error(), {});
        return;
    }
    if (*also_cname == true) {
        hints.ai_flags |= AI_CANONNAME;
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;

    SharedPtr<Message> message{std::make_shared<Message>()};
    message->queries.push_back(query);

    getaddrinfo_async<getaddrinfo, inet_ntop>(name, hints, reactor, logger,
            [ message = std::move(message), cb = std::move(cb) ](
                    Error error, std::vector<Answer> answers) {
                message->answers = std::move(answers);
                cb(error, message);
            });
}

} // namespace dns
} // namespace mk
#endif
