// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_DNS_QUERY_HPP
#define PRIVATE_LIBEVENT_DNS_QUERY_HPP

#include "private/common/mock.hpp"
#include "private/common/utils.hpp"
#include "private/dns/utils.hpp"
#include "../net/utils.hpp"

#include <event2/dns.h>

#include <cassert>
#include <new>
#include <memory>
#include <limits.h>
#include <type_traits>

extern "C" {

static inline void mk_evdns_handle_resolve(int code, char type, int count,
                                           int ttl, void *addresses,
                                           void *opaque);

} // extern "C"

struct evdns_base;
namespace mk {
namespace libevent {

using namespace mk::dns;

inline Error dns_error(int code) {
    Error err;

    //
    // Here we map evdns error codes to specific Errors.
    //
    // We start with errors specified in RFC 1035 (see also event2/dns.h).
    //

    if (code == DNS_ERR_NONE) {
        err = NoError();

    } else if (code == DNS_ERR_FORMAT) {
        // The name server was unable to interpret the query
        err = FormatError();

    } else if (code == DNS_ERR_SERVERFAILED) {
        // The name server was unable to process this query due to a
        // problem with the name server
        err = ServerFailedError();

    } else if (code == DNS_ERR_NOTEXIST) {
        // The domain name does not exist
        err = NotExistError();

    } else if (code == DNS_ERR_NOTIMPL) {
        // The name server does not support the requested kind of query
        err = dns::NotImplementedError();

    } else if (code == DNS_ERR_REFUSED) {
        // The name server refuses to perform the specified operation
        // for policy reasons
        err = RefusedError();

        //
        // The following are evdns specific errors
        //

    } else if (code == DNS_ERR_TRUNCATED) {
        // The reply was truncated or ill-formatted
        err = TruncatedError();

    } else if (code == DNS_ERR_UNKNOWN) {
        // An unknown error occurred
        err = UnknownError();

    } else if (code == DNS_ERR_TIMEOUT) {
        // Communication with the server timed out
        err = TimeoutError();

    } else if (code == DNS_ERR_SHUTDOWN) {
        // The request was canceled because the DNS subsystem was shut down.
        err = ShutdownError();

    } else if (code == DNS_ERR_CANCEL) {
        // The request was canceled via a call to evdns_cancel_request
        err = CancelError();

    } else if (code == DNS_ERR_NODATA) {
        // There were no answers and no error condition in the DNS packet.
        // This can happen when you ask for an address that exists, but
        // a record type that doesn't.
        err = NoDataError();

    } else {
        // Safety net - should really not happen
        err = GenericError();
    }

    return err;
}

class QueryContext : public NonMovable, public NonCopyable {
  public:
    ~QueryContext() {
        if (base != nullptr) {
            evdns_base_free(base, 0);
        }
    }

    double ticks;

    evdns_base *base = nullptr;

    SharedPtr<Message> message;
    Callback<Error, SharedPtr<Message>> callback;

    SharedPtr<Logger> logger = Logger::global();
    SharedPtr<Reactor> reactor = Reactor::global();

    QueryContext(evdns_base *b, Callback<Error, SharedPtr<Message>> c,
            SharedPtr<Message> m, SharedPtr<Logger> l = Logger::global(),
            SharedPtr<Reactor> r = Reactor::global()) {
        base = b;
        callback = c;
        message = m;
        ticks = mk::time_now();
        logger = l;
        reactor = r;
    }
};

struct evdns_base_deleter {
    void operator()(evdns_base *p) {
        constexpr int fail_requests = 1;
        evdns_base_free(p, fail_requests);
    }
};

struct evaddrinfo_deleter {
    void operator()(evutil_addrinfo *p) { evutil_freeaddrinfo(p); }
};

using evdns_base_uptr = std::unique_ptr<evdns_base, evdns_base_deleter>;
using evaddrinfo_uptr = std::unique_ptr<evutil_addrinfo, evaddrinfo_deleter>;

template <MK_MOCK(evdns_base_new), MK_MOCK(evdns_base_nameserver_sockaddr_add),
          typename evdns_base_uptr = evdns_base_uptr,
          MK_MOCK(evdns_base_set_option)>
static inline evdns_base *
create_evdns_base(Settings settings, SharedPtr<Reactor> reactor = Reactor::global()) {

    event_base *evb = reactor->get_event_base();
    const int initialize_nameservers = settings.count("dns/nameserver") ? 0 : 1;
    evdns_base_uptr base(evdns_base_new(evb, initialize_nameservers));
    if (!base) {
        throw std::bad_alloc();
    }

    if (!initialize_nameservers) {
        // libevent can't handle link-local IPv6 nameserver in
        // evdns_base_nameserver_ip_add, and there is no way to parse alike
        // addresses in platform-independent way, so that's why getaddrinfo().
        // fe80::227:22ff:fe45:3a92%wlan0 -- Android
        // fe80::227:22ff:fe45:3a92%wlp1s0 -- Ubuntu Linux
        evutil_addrinfo hints = {};
        hints.ai_family = PF_UNSPEC;
        hints.ai_flags = EVUTIL_AI_NUMERICSERV | EVUTIL_AI_NUMERICHOST;
        hints.ai_socktype = SOCK_DGRAM;
        evutil_addrinfo *res = nullptr;
        std::string port = settings.get("dns/port", std::string{"53"});
        const int eaierr = evutil_getaddrinfo(
              settings["dns/nameserver"].c_str(), port.c_str(), &hints, &res);
        evaddrinfo_uptr ai(res);
        if (eaierr) {
            throw std::runtime_error(
                  std::string("Cannot parse server address: ") +
                  evutil_gai_strerror(eaierr));
        }

        constexpr unsigned unused = 0; // libevent API does not use it :-/
        const int nserr = evdns_base_nameserver_sockaddr_add(
              base.get(), ai->ai_addr, ai->ai_addrlen, unused);
        if (nserr != 0) {
            throw std::runtime_error("Cannot set server address: " +
                                     std::to_string(nserr));
        }
    }

    if (settings.find("dns/attempts") != settings.end() &&
        evdns_base_set_option(base.get(), "attempts",
                              settings["dns/attempts"].c_str()) != 0) {
        throw std::runtime_error("Cannot set 'attempts' option");
    }
    if (settings.find("dns/timeout") != settings.end() &&
        evdns_base_set_option(base.get(), "timeout",
                              settings["dns/timeout"].c_str()) != 0) {
        throw std::runtime_error("Cannot set 'timeout' option");
    }

    // By default we don't randomize the query's case
    // XXX check that randomize case is a valid value
    std::string randomiz{"0"};
    if (settings.find("dns/randomize_case") != settings.end()) {
        randomiz = settings["dns/randomize_case"];
    }
    if (evdns_base_set_option(base.get(), "randomize-case", randomiz.c_str()) !=
        0) {
        throw std::runtime_error("Cannot set 'randomize-case' option");
    }

    return base.release();
}

template <MK_MOCK(inet_ntop)>
static inline std::vector<Answer>
build_answers_evdns(int code, char type, int count, int ttl, void *addresses,
                    SharedPtr<Logger> logger = Logger::global()) {

    std::vector<Answer> answers;

    if (code != DNS_ERR_NONE) {
        logger->debug("dns: request failed: %d", code);
        // do not process the results if there was an error

    } else if (type == DNS_PTR) {
        logger->debug("dns: PTR");
        Answer answer;
        answer.code = code;
        answer.ttl = ttl;
        answer.type = MK_DNS_TYPE_PTR;
        // Note: cast magic copied from libevent regress tests
        answer.hostname = std::string(*(char **)addresses);
        logger->debug("dns: adding %s", answer.hostname.c_str());
        answers.push_back(answer);
    } else if (type == DNS_IPv4_A || type == DNS_IPv6_AAAA) {

        int family;
        int size;
        char string[128]; // Is wide enough (max. IPv6 length is 45 chars)

        if (type == DNS_IPv4_A) {
            family = PF_INET;
            size = 4;
            logger->debug("dns: IPv4");
        } else {
            family = PF_INET6;
            size = 16;
            logger->debug("dns: IPv6");
        }

        //
        // Note: make sure in advance `i * size` won't overflow,
        // this is here only for robustness.
        //
        if (count >= 0 && count <= INT_MAX / size + 1) {

            for (auto i = 0; i < count; ++i) {
                // Note: address already in network byte order
                if (inet_ntop(family, (char *)addresses + i * size, string,
                              sizeof(string)) == NULL) {
                    logger->warn("dns: unexpected inet_ntop failure");
                    throw std::runtime_error("Unexpected inet_ntop failure");
                }
                Answer answer;
                answer.code = code;
                answer.ttl = ttl;
                if (family == PF_INET) {
                    answer.ipv4 = string;
                    answer.type = MK_DNS_TYPE_A;
                } else if (family == PF_INET6) {
                    answer.ipv6 = string;
                    answer.type = MK_DNS_TYPE_AAAA;
                }
                logger->debug("dns: adding '%s'", string);
                answers.push_back(answer);
            }

        } else {
            logger->warn("dns: too many addresses");
            throw std::runtime_error("We got more responses than the maximum "
                                     "integer size. Something is very wrong.");
        }

    } else {
        logger->warn("dns: invalid response type");
        throw std::runtime_error("Invalid response type.");
    }
    return answers;
}

static inline void dns_callback(int code, char type, int count, int ttl,
                                void *addresses, QueryContext *context) {
    context->message->error_code = code;

    switch (code) {
    case DNS_ERR_NONE:
    case DNS_ERR_FORMAT:
    case DNS_ERR_SERVERFAILED:
    case DNS_ERR_NOTEXIST:
    case DNS_ERR_NOTIMPL:
    case DNS_ERR_REFUSED:
    case DNS_ERR_TRUNCATED:
    case DNS_ERR_NODATA:
        context->message->rtt = mk::time_now() - context->ticks;
        break;
    default:
        context->message->rtt = 0.0;
        break;
    }

    context->message->answers = build_answers_evdns(code, type, count, ttl,
                                                    addresses, context->logger);
    context->reactor->with_current_data_usage([&context](DataUsage &du) {
        if (context->message->queries.size() < 1) {
            throw std::runtime_error("malformed message");
        }
        auto query = context->message->queries[0];
        dns::estimate_data_usage(du, query.name, context->message->answers,
                context->logger);
    });
    try {
        if (context->message->error_code != DNS_ERR_NONE) {
            context->callback(dns_error(context->message->error_code),
                              context->message);
        } else {
            context->callback(NoError(), context->message);
        }
    } catch (const Error &) {
        // suppress Error exceptions because we don't want this kind
        // of exception to terminate the program
    }
    delete context;
}

template <MK_MOCK(evdns_base_free), MK_MOCK(evdns_base_resolve_ipv4),
          MK_MOCK(evdns_base_resolve_ipv6), MK_MOCK(evdns_base_resolve_reverse),
          MK_MOCK(evdns_base_resolve_reverse_ipv6), MK_MOCK(inet_pton)>
void query(QueryClass dns_class, QueryType dns_type, std::string name,
           Callback<Error, SharedPtr<Message>> cb, Settings settings,
           SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    /*
     * When running OONI tests, we're interested to know not only the IPs
     * associated with a specific name, but also to the CNAME.
     *
     * This is not yet implemented in the libevent engine.
     */
    ErrorOr<bool> also_cname = settings.get("dns/resolve_also_cname", false);
    if (!also_cname) {
        cb(also_cname.as_error(), {});
        return;
    }
    if (*also_cname == true) {
        cb(mk::NotImplementedError(), {});
        return;
    }

    SharedPtr<Message> message{std::make_shared<Message>()};
    Query query;
    evdns_base *base;

    try {
        base = create_evdns_base(settings, reactor);
    } catch (std::runtime_error &) {
        cb(GenericError(), {}); // TODO: refine error thrown here
        return;
    } catch (std::bad_alloc &) {
        throw; // Let this propagate as we can do nothing
    }

    if (dns_class != MK_DNS_CLASS_IN) {
        evdns_base_free(base, 1);
        cb(UnsupportedClassError(), {});
        return;
    }

    // Allow PTR queries
    if (dns_type == MK_DNS_TYPE_PTR) {
        std::string s;
        if ((s = net::unreverse_ipv4(name)) != "") {
            dns_type = MK_DNS_TYPE_REVERSE_A;
            name = s;
        } else if ((s = net::unreverse_ipv6(name)) != "") {
            dns_type = MK_DNS_TYPE_REVERSE_AAAA;
            name = s;
        } else {
            evdns_base_free(base, 1);
            cb(InvalidNameForPTRError(), {});
            return;
        }
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;
    message->queries.push_back(query);

    //
    // Note: evdns_base_resolve_xxx() return a evdns_request
    // object that may be used to cancel the request. Yet, the
    // semantic of cancelling a request is such that evdns
    // could invoke the callback of a pending request if such
    // request was cancelled when its corresponding response
    // was already pending. This seems to be confirmed by
    // comments in libevent's evdns.c:
    //
    //     This does nothing if the request's callback is
    //     already running (pending_cb is set)
    //
    // For the above reason this class does not explicitly
    // cancel pending evdns requests and uses the `cancelled`
    // variable to keep track of cancelled requests.
    //
    if (dns_type == MK_DNS_TYPE_A) {
        logger->debug("dns query: IN A %s", name.c_str());
        QueryContext *context = new QueryContext(base, cb, message,
                logger, reactor);
        if (evdns_base_resolve_ipv4(base, name.c_str(), DNS_QUERY_NO_SEARCH,
                                    mk_evdns_handle_resolve,
                                    context) == nullptr) {
            delete context;
            cb(ResolverError(), {});
        }
        return;
    }

    if (dns_type == MK_DNS_TYPE_AAAA) {
        logger->debug("dns query: IN AAAA %s", name.c_str());
        QueryContext *context = new QueryContext(base, cb, message,
                logger, reactor);
        if (evdns_base_resolve_ipv6(base, name.c_str(), DNS_QUERY_NO_SEARCH,
                                    mk_evdns_handle_resolve,
                                    context) == nullptr) {
            delete context;
            cb(ResolverError(), {});
        }
        return;
    }

    if (dns_type == MK_DNS_TYPE_REVERSE_A) {
        logger->debug("dns query: IN REVERSE_A %s", name.c_str());
        in_addr netaddr;
        if (inet_pton(AF_INET, name.c_str(), &netaddr) != 1) {
            evdns_base_free(base, 1);
            cb(InvalidIPv4AddressError(), {});
            return;
        }

        QueryContext *context = new QueryContext(base, cb, message,
                logger, reactor);
        if (evdns_base_resolve_reverse(base, &netaddr, DNS_QUERY_NO_SEARCH,
                                       mk_evdns_handle_resolve,
                                       context) == nullptr) {
            delete context;
            cb(ResolverError(), {});
        }
        return;
    }

    if (dns_type == MK_DNS_TYPE_REVERSE_AAAA) {
        logger->debug("dns query: IN REVERSE_AAAA %s", name.c_str());
        in6_addr netaddr;
        if (inet_pton(AF_INET6, name.c_str(), &netaddr) != 1) {
            evdns_base_free(base, 1);
            cb(InvalidIPv6AddressError(), {});
            return;
        }

        QueryContext *context = new QueryContext(base, cb, message,
                logger, reactor);
        if (evdns_base_resolve_reverse_ipv6(base, &netaddr, DNS_QUERY_NO_SEARCH,
                                            mk_evdns_handle_resolve,
                                            context) == nullptr) {
            delete context;
            cb(ResolverError(), {});
            return;
        }
        return;
    }

    evdns_base_free(base, 1);
    cb(UnsupportedTypeError(), {});
}

} // namespace libevent
} // namespace mk

static inline void mk_evdns_handle_resolve(int code, char type, int count,
                                           int ttl, void *addresses,
                                           void *opaque) {
    using namespace mk::libevent;
    auto context = static_cast<QueryContext *>(opaque);
    dns_callback(code, type, count, ttl, addresses, context);
}
#endif
