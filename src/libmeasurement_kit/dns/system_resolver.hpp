// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SYSTEM_RESOLVER_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SYSTEM_RESOLVER_HPP

#include <measurement_kit/dns.hpp>

#include "../dns/getaddrinfo_async.hpp"

namespace mk {
namespace dns {

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void system_resolver(QueryClass dns_class, QueryType dns_type, std::string name,
                     Var<Reactor> reactor, Var<Logger> logger,
                     Callback<Error, Var<Message>> cb) {
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
        reactor->call_soon([=]() { cb(UnsupportedClassError(), nullptr); });
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
        reactor->call_soon([=]() { cb(UnsupportedTypeError(), nullptr); });
        return;
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;

    Var<Message> message{new Message};
    message->queries.push_back(query);

    getaddrinfo_async<getaddrinfo, inet_ntop>(
        name, hints, reactor, logger, Worker::global(),
        [ message = std::move(message),
          cb = std::move(cb) ](Error error, std::vector<Answer> answers) {
            message->answers = std::move(answers);
            cb(error, message);
        });
}

} // namespace dns
} // namespace mk
#endif
