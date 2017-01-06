// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SYSTEM_RESOLVER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SYSTEM_RESOLVER_IMPL_HPP

#include "../dns/system_resolver.hpp"

#include <cassert>
#include <future>

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
        memset(&hints, 0, sizeof (hints));
    }
};

class addrinfo_deleter {
  public:
    void operator()(addrinfo *ptr) {
        if (ptr) {
            freeaddrinfo(ptr);
        }
    }
};
using addrinfo_uptr = std::unique_ptr<addrinfo, addrinfo_deleter>;

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void resolve_async_impl(ResolverContext *context) {
    std::unique_ptr<ResolverContext> ctx(context);
    Callback<Error, Var<Message>> callback = ctx->cb;
    Var<Message> message = ctx->message;
    addrinfo *sip = nullptr;

    int error = getaddrinfo(ctx->name.c_str(), nullptr, &ctx->hints, &sip);
    ctx->logger->log(MK_LOG_DEBUG2, "getaddrinfo result: %d", error);
    if (error) {
        Error resolver_error;
        switch (error) {
        case EAI_AGAIN:
            resolver_error = TemporaryFailureError();
            break;
        case EAI_BADFLAGS:
            resolver_error = InvalidFlagsValueError();
            break;
#ifdef EAI_BADHINTS  // Not always available
        case EAI_BADHINTS:
            resolver_error = InvalidHintsValueError();
            break;
#endif
        case EAI_FAIL:
            resolver_error = NonRecoverableFailureError();
            break;
        case EAI_FAMILY:
            resolver_error = NotSupportedAIFamilyError();
            break;
        case EAI_MEMORY:
            resolver_error = MemoryAllocationFailureError();
            break;
        case EAI_NONAME:
            resolver_error = HostOrServiceNotProvidedOrNotKnownError();
            break;
        case EAI_OVERFLOW:
            resolver_error = ArgumentBufferOverflowError();
            break;
#ifdef EAI_PROTOCOL  // Not always available
        case EAI_PROTOCOL:
            resolver_error = UnknownResolvedProtocolError();
            break;
#endif
        case EAI_SERVICE:
            resolver_error = NotSupportedServnameError();
            break;
        case EAI_SOCKTYPE:
            resolver_error = NotSupportedAISocktypeError();
            break;
        default:
            resolver_error = ResolverError();
            break;
        }
        ctx->reactor->call_soon([=]() { callback(resolver_error, nullptr); });
        return;
    }
    assert(sip != nullptr);
    addrinfo_uptr servinfo(sip);

    void *addr_ptr;
    char abuf[128];
    for (addrinfo *p = servinfo.get(); p != nullptr; p = p->ai_next) {
        Answer answer;
        answer.name = ctx->name;
        answer.qclass = ctx->dns_class;
        assert(p->ai_family == AF_INET or p->ai_family == AF_INET6);
        if (p->ai_family == AF_INET) {
            answer.type = MK_DNS_TYPE_A;
            addr_ptr = &((sockaddr_in *)p->ai_addr)->sin_addr;
        } else if (p->ai_family == AF_INET6) {
            answer.type = MK_DNS_TYPE_AAAA;
            addr_ptr = &((sockaddr_in6 *)p->ai_addr)->sin6_addr;
        } else {
            // Added branch to avoid g++ compiler warning
            throw std::runtime_error("internal error");
        }
        if (p->ai_canonname != nullptr) {
            answer.hostname = p->ai_canonname;
        }
        if (inet_ntop(p->ai_family, addr_ptr, abuf, sizeof(abuf)) == nullptr) {
            ctx->logger->warn("dns: unexpected inet_ntop failure");
            ctx->reactor->call_soon(
                [=]() { callback(InetNtopFailureError(), nullptr); });
            return;
        }
        if (p->ai_family == AF_INET) {
            answer.ipv4 = std::string(abuf);
        } else if (p->ai_family == AF_INET6) {
            answer.ipv6 = std::string(abuf);
        }
        message->answers.push_back(answer);
    }
    ctx->reactor->call_soon([=]() { callback(NoError(), message); });
}

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void system_resolver_impl(QueryClass dns_class, QueryType dns_type,
        std::string name, Callback<Error, Var<Message>> cb, Settings settings,
        Var<Reactor> reactor, Var<Logger> logger) {
    std::unique_ptr<ResolverContext> ctx(new ResolverContext(
        dns_class, dns_type, name, cb, settings, reactor, logger));
    Query query;
    /*
     * Note: here we pass empty flags. It used to be AI_ALL | AI_V4MAPPED but
     * Android did not like it and, honestly, I do not think having back v4
     * mapped addresses would be super useful to us (or in general) given that
     * when we want to connect we usually connect trying to resolve AAAA and
     * A at the same time. Keeping the feature conditionally may lead to
     * odd behaviors in corner cases and I'd like to avoid future headaches.
     */
    ctx->hints.ai_socktype = SOCK_STREAM;

    if (dns_class != MK_DNS_CLASS_IN) {
        reactor->call_soon([=]() { cb(UnsupportedClassError(), nullptr); });
        return;
    }

    if (dns_type == MK_DNS_TYPE_A) {
        ctx->hints.ai_family = AF_INET;
    } else if (dns_type == MK_DNS_TYPE_AAAA) {
        ctx->hints.ai_family = AF_INET6;
    } else if (dns_type == MK_DNS_TYPE_CNAME) {
        ctx->hints.ai_family = AF_UNSPEC;
        ctx->hints.ai_flags |= AI_CANONNAME;
    } else {
        reactor->call_soon([=]() { cb(UnsupportedTypeError(), nullptr); });
        return;
    }

    query.type = dns_type;
    query.qclass = dns_class;
    query.name = name;

    ctx->message->queries.push_back(query);

    std::async(std::launch::async, resolve_async_impl<getaddrinfo, inet_ntop>,
               ctx.release());
}

} // namespace dns
} // namespace mk
#endif
