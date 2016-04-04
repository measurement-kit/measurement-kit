// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>

#include "src/common/utils.hpp"
#include <measurement_kit/common.hpp>

#include <event2/dns.h>

#include <cassert>
#include <functional>
#include <iosfwd>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

extern "C" {
    void handle_resolve(int code, char type, int count, int ttl,
            void *addresses, void *opaque);
}

struct evdns_base;
namespace mk {
namespace dns {

class QueryContext {
  public:
    ~QueryContext() {
        if (base != nullptr) {
            evdns_base_free(base, 0);
        }
    }

    double ticks;

    evdns_base *base = nullptr;

    Message message;
    std::function<void(Error, Message)> callback;

    QueryContext(
            evdns_base *b, std::function<void(Error, Message)> c, Message m) {
        base = b;
        callback = c;
        message = m;
        ticks = mk::time_now();
    }
};


static inline evdns_base *create_evdns_base(
        Settings settings, Poller *poller = mk::get_global_poller()) {

    evdns_base *base;
    event_base *evb = poller->get_event_base();

    if (settings.find("nameserver") != settings.end()) {
        if ((base = evdns_base_new(evb, 0)) == nullptr) {
            throw std::bad_alloc();
        }
        if (evdns_base_nameserver_ip_add(
                    base, settings["nameserver"].c_str()) != 0) {
            evdns_base_free(base, 1);
            throw std::runtime_error("Cannot set server address");
        }
    } else if ((base = evdns_base_new(evb, 1)) == nullptr) {
        throw std::bad_alloc();
    }

    if (settings.find("attempts") != settings.end() &&
            evdns_base_set_option(
                    base, "attempts", settings["attempts"].c_str()) != 0) {
        evdns_base_free(base, 1);
        throw std::runtime_error("Cannot set 'attempts' option");
    }
    if (settings.find("timeout") != settings.end() &&
            evdns_base_set_option(
                    base, "timeout", settings["timeout"].c_str()) != 0) {
        evdns_base_free(base, 1);
        throw std::runtime_error("Cannot set 'timeout' option");
    }

    // By default we don't randomize the query's case
    std::string randomiz{"0"};
    if (settings.find("randomize_case") != settings.end()) {
        randomiz = settings["randomize_case"];
    }
    if (evdns_base_set_option(base, "randomize-case", randomiz.c_str()) != 0) {
        evdns_base_free(base, 1);
        throw std::runtime_error("Cannot set 'randomize-case' option");
    }

    return base;
}

static inline std::vector<Answer> build_answers_evdns(
        int code, char type, int count, int ttl, void *addresses) {

    Logger *logger = Logger::global();

    std::vector<Answer> answers;

    if (code != DNS_ERR_NONE) {
        logger->info("dns - request failed: %d", code);
        // do not process the results if there was an error

    } else if (type == DNS_PTR) {
        logger->info("dns - PTR");
        Answer answer;
        answer.code = code;
        answer.ttl = ttl;
        answer.type = QueryTypeId::PTR;
        // Note: cast magic copied from libevent regress tests
        answer.hostname = std::string(*(char **)addresses);
        logger->info("dns - adding %s", answer.hostname.c_str());
        answers.push_back(answer);
    } else if (type == DNS_IPv4_A || type == DNS_IPv6_AAAA) {

        int family;
        int size;
        char string[128]; // Is wide enough (max. IPv6 length is 45 chars)

        if (type == DNS_IPv4_A) {
            family = PF_INET;
            size = 4;
            logger->info("dns - IPv4");
        } else {
            family = PF_INET6;
            size = 16;
            logger->info("dns - IPv6");
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
                    logger->warn("dns - unexpected inet_ntop failure");
                    throw std::runtime_error("Unexpected inet_ntop failure");
                }
                Answer answer;
                answer.code = code;
                answer.ttl = ttl;
                if (family == PF_INET) {
                    answer.ipv4 = string;
                    answer.type = QueryTypeId::A;
                } else if (family == PF_INET6) {
                    answer.ipv6 = string;
                    answer.type = QueryTypeId::AAAA;
                }
                logger->info("dns - adding '%s'", string);
                answers.push_back(answer);
            }

        } else {
            logger->warn("dns - too many addresses");
            throw std::runtime_error("We got more responses than the maximum "
                                     "integer size. Something is very wrong.");
        }

    } else {
        logger->warn("dns - invalid response type");
        throw std::runtime_error("Invalid response type.");
    }
    return answers;
}

static void dns_callback(int code, char type, int count, int ttl,
        void *addresses, QueryContext *context) {
    context->message.error_code = code;

    switch (code) {
    case DNS_ERR_NONE:
    case DNS_ERR_FORMAT:
    case DNS_ERR_SERVERFAILED:
    case DNS_ERR_NOTEXIST:
    case DNS_ERR_NOTIMPL:
    case DNS_ERR_REFUSED:
    case DNS_ERR_TRUNCATED:
    case DNS_ERR_NODATA:
        context->message.rtt = mk::time_now() - context->ticks;
        break;
    default:
        context->message.rtt = 0.0;
        break;
    }

    context->message.answers =
            build_answers_evdns(code, type, count, ttl, addresses);
    if (context->message.error_code != DNS_ERR_NONE) {
        context->callback(mk::dns::dns_error(context->message.error_code),
                context->message);
    } else {
        context->callback(NoError(), context->message);
    }

    delete context;
}

void query_debug (QueryClass dns_class, QueryType dns_type, std::string name,
        std::function<void(Error, Message)> cb, Settings settings,
        Poller *poller) {

    Message message;
    Query query;
    evdns_base *base;

    try {
        base = create_evdns_base(settings, poller);
    } catch (std::runtime_error &) {
        cb(GenericError(), nullptr); // TODO: refine error thrown here
        return;
    } catch (std::bad_alloc &) {
        throw; // Let this propagate as we can do nothing
    }

    if (dns_class != QueryClassId::IN) {
        evdns_base_free(base, 1);
        cb(UnsupportedClassError(), nullptr);
        return;
    }

    // Allow PTR queries
    if (dns_type == QueryTypeId::PTR) {
        std::string s;
        if ((s = mk::unreverse_ipv4(name)) != "") {
            dns_type = QueryTypeId::REVERSE_A;
            name = s;
        } else if ((s = mk::unreverse_ipv6(name)) != "") {
            dns_type = QueryTypeId::REVERSE_AAAA;
            name = s;
        } else {
            evdns_base_free(base, 1);
            cb(InvalidNameForPTRError(), nullptr);
            return;
        }
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;
    message.queries.push_back(query);

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
    if (dns_type == QueryTypeId::A) {
        if (evdns_base_resolve_ipv4(base, name.c_str(), DNS_QUERY_NO_SEARCH,
                    handle_resolve,
                    new QueryContext(base, cb, message)) == nullptr) {
            evdns_base_free(base, 1);
            cb(ResolverError(), nullptr);
        }
        return;
    }

    if (dns_type == QueryTypeId::AAAA) {
        if (evdns_base_resolve_ipv6(base, name.c_str(), DNS_QUERY_NO_SEARCH,
                    handle_resolve,
                    new QueryContext(base, cb, message)) == nullptr) {
            evdns_base_free(base, 1);
            cb(ResolverError(), nullptr);
        }
        return;
    }

    if (dns_type == QueryTypeId::REVERSE_A) {
        in_addr netaddr;
        if (inet_pton(AF_INET, name.c_str(), &netaddr) != 1) {
            evdns_base_free(base, 1);
            cb(InvalidIPv4AddressError(), nullptr);
            return;
        }

        if (evdns_base_resolve_reverse(base, &netaddr, DNS_QUERY_NO_SEARCH,
                    handle_resolve,
                    new QueryContext(base, cb, message)) == nullptr) {
            evdns_base_free(base, 1);
            cb(ResolverError(), nullptr);
        }
        return;
    }

    if (dns_type == QueryTypeId::REVERSE_AAAA) {
        in6_addr netaddr;
        if (inet_pton(AF_INET6, name.c_str(), &netaddr) != 1) {
            evdns_base_free(base, 1);
            cb(InvalidIPv6AddressError(), nullptr);
            return;
        }

        if (evdns_base_resolve_reverse_ipv6(base, &netaddr, DNS_QUERY_NO_SEARCH,
                    handle_resolve,
                    new QueryContext(base, cb, message)) == nullptr) {
            evdns_base_free(base, 1);
            cb(ResolverError(), nullptr);
            return;
        }
        return;
    }

    evdns_base_free(base, 1);
    cb(UnsupportedTypeError(), nullptr);
}

} // namespace dns
} // namespace mk
