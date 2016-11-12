// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>
#include <measurement_kit/portable/sys/socket.h>
#include <measurement_kit/portable/sys/types.h>

#include <cassert>
#include <future>

#include <netdb.h>

namespace mk {
namespace dns {

class ResolverContext {
  public:
    QueryClass dns_class;
    QueryType dns_type;
    std::string name;
    Callback<Error, Var<Message>> cb;
    Settings settings;
    Var<Reactor> reactor;
    Var<Logger> logger;

    addrinfo hints;
    addrinfo *servinfo = nullptr;

    Var<Message> message{new Message};

    ResolverContext(QueryClass dns_c, QueryType dns_t, std::string n,
                    Callback<Error, Var<Message>> c, Settings s, Var<Reactor> r,
                    Var<Logger> l) {
        dns_class = dns_c;
        dns_type = dns_t;
        name = n;
        cb = c;
        settings = s;
        reactor = r;
        logger = l;
        memset(&hints, 0, sizeof(hints));
    }

    ~ResolverContext() {
        if (servinfo != nullptr) {
            freeaddrinfo(servinfo);
        }
    }
};

template<MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
static inline void resolve_async(ResolverContext *context) {
    std::unique_ptr<ResolverContext> ctx(context);
    Callback<Error, Var<Message>> callback = ctx->cb;
    Var<Message> message_copy;

    int error = getaddrinfo(ctx->name.c_str(), nullptr, &(ctx->hints),
                            &(ctx->servinfo));
    if (error) {
        // check the error variable and return the correct error
        ctx->logger->warn(gai_strerror(error));
        ctx->reactor->call_soon(
            [callback]() { callback(ResolverError(), nullptr); });
        return;
    }

    assert(ctx->servinfo != nullptr);

    void *addr_ptr;
    char address[128];
    std::vector<Answer> answers;
    for (addrinfo *p = ctx->servinfo; p != nullptr; p = p->ai_next) {
        Answer answer;
        answer.name = ctx->name;
        answer.qclass = ctx->dns_class;
        if (p->ai_family == AF_INET) {
            answer.type = QueryTypeId::A;
            addr_ptr = &((sockaddr_in *)p->ai_addr)->sin_addr;
        } else if (p->ai_family == AF_INET6) {
            answer.type = QueryTypeId::AAAA;
            addr_ptr = &((sockaddr_in6 *)p->ai_addr)->sin6_addr;
        } else {
            ctx->reactor->call_soon(
                [callback]() { callback(ResolverError(), nullptr); });
            return;
        }
        if (inet_ntop(p->ai_family, addr_ptr, address, sizeof(address)) ==
            NULL) {
            ctx->logger->warn("dns: unexpected inet_ntop failure");
            throw std::runtime_error("Unexpected inet_ntop failure");
        }
        if (p->ai_family == AF_INET) {
            answer.ipv4 = std::string(address);
        } else if (p->ai_family == AF_INET6) {
            answer.ipv6 = std::string(address);
        }
        answers.push_back(answer);
    }
    ctx->message->answers = answers;
    message_copy = ctx->message;
    ctx->reactor->call_soon(
        [callback, message_copy]() { callback(NoError(), message_copy); });
}

template<MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
inline void system_resolver(QueryClass dns_class, QueryType dns_type,
                            std::string name, Callback<Error, Var<Message>> cb,
                            Settings settings, Var<Reactor> reactor,
                            Var<Logger> logger) {
    std::unique_ptr<ResolverContext> ctx(new ResolverContext(dns_class, dns_type, name, cb,
                                               settings, reactor, logger));
    Query query;
    ctx->hints.ai_flags = AI_ADDRCONFIG;
    ctx->hints.ai_socktype = SOCK_STREAM;

    if (dns_class != QueryClassId::IN) {
        reactor->call_soon([=]() {
            cb(UnsupportedClassError(), nullptr);
        });
        return;
    }

    if (dns_type == QueryTypeId::A) {
        ctx->hints.ai_family = AF_INET;
    } else if (dns_type == QueryTypeId::AAAA) {
        ctx->hints.ai_family = AF_INET6;
    } else {
        reactor->call_soon([=]() {
            cb(UnsupportedClassError(), nullptr);
        });
        return;
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;

    ctx->message->queries.push_back(query);

    std::async(std::launch::async, resolve_async<getaddrinfo, inet_ntop>, ctx.release());
}

} // namespace dns
} // namespace mk
