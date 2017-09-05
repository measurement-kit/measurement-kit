// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_DNS_GETADDRINFO_ASYNC_HPP
#define PRIVATE_DNS_GETADDRINFO_ASYNC_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

#define GETADDRINFO_ASYNC_MAP_ERROR                                            \
    XX(0, NoError)                                                             \
    XX(EAI_AGAIN, TemporaryFailureError)                                       \
    XX(EAI_BADFLAGS, InvalidFlagsValueError)                                   \
    XX(EAI_FAIL, NonRecoverableFailureError)                                   \
    XX(EAI_FAMILY, NotSupportedAIFamilyError)                                  \
    XX(EAI_MEMORY, MemoryAllocationFailureError)                               \
    XX(EAI_NONAME, HostOrServiceNotProvidedOrNotKnownError)                    \
    XX(EAI_OVERFLOW, ArgumentBufferOverflowError)                              \
    XX(EAI_SERVICE, NotSupportedServnameError)                                 \
    XX(EAI_SOCKTYPE, NotSupportedAISocktypeError)

inline Error getaddrinfo_async_map_error(int error) {
    switch (error) {
#define XX(code_, class_name_)                                                 \
    case code_:                                                                \
        return class_name_();
        GETADDRINFO_ASYNC_MAP_ERROR
#undef XX
#ifdef EAI_BADHINTS // Not always available
    case EAI_BADHINTS:
        return InvalidHintsValueError();
#endif
#ifdef EAI_PROTOCOL // Not always available
    case EAI_PROTOCOL:
        return UnknownResolvedProtocolError();
#endif
    default:
        break;
    }
    return ResolverError();
}

template <MK_MOCK(inet_ntop)>
std::vector<Answer> getaddrinfo_async_parse_response(const std::string &name,
                                                     addrinfo *rp) {
    std::vector<Answer> answers;
    for (addrinfo *p = rp; p != nullptr; p = p->ai_next) {
        Answer answer;
        answer.name = name;
        answer.qclass = "IN";
        void *aptr = nullptr;
        if (p->ai_family == AF_INET) {
            answer.type = MK_DNS_TYPE_A;
            aptr = &((sockaddr_in *)p->ai_addr)->sin_addr;
        } else if (p->ai_family == AF_INET6) {
            answer.type = MK_DNS_TYPE_AAAA;
            aptr = &((sockaddr_in6 *)p->ai_addr)->sin6_addr;
        } else {
            throw GenericError(); /* Avoid g++ warning */
        }
        if (p->ai_canonname != nullptr) {
            answer.hostname = p->ai_canonname;
        }
        char abuf[128];
        if (inet_ntop(p->ai_family, aptr, abuf, sizeof(abuf)) == nullptr) {
            throw InetNtopFailureError();
        }
        if (p->ai_family == AF_INET) {
            answer.ipv4 = abuf;
        } else if (p->ai_family == AF_INET6) {
            answer.ipv6 = abuf;
        } else {
            /* Case excluded above */;
        }
        answers.push_back(answer);
    }
    return answers;
}

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void getaddrinfo_async(std::string name, addrinfo hints, Var<Reactor> reactor,
                       Var<Logger> logger,
                       Callback<Error, std::vector<Answer>> cb) {
    /*
     * Move everything down such that there is always just one function in
     * one specific thread having ownership of the state
     */
    reactor->run_in_background_thread([
        name = std::move(name), hints = std::move(hints),
        reactor = std::move(reactor), logger = std::move(logger),
        cb = std::move(cb)
    ]() {
        addrinfo *rp = nullptr;
        Error error = getaddrinfo_async_map_error(
            getaddrinfo(name.c_str(), nullptr, &hints, &rp));
        logger->debug("getaddrinfo('%s') => error: code=%d, reason='%s'",
                      name.c_str(), error.code, error.as_ooni_error().c_str());
        std::vector<Answer> answers;
        if (!error && rp != nullptr) {
            try {
                answers = getaddrinfo_async_parse_response<inet_ntop>(name, rp);
            } catch (const Error &err) {
                error = std::move(err);
            }
        }
        if (rp != nullptr) {
            freeaddrinfo(rp);
        }
        /*
         * Pass through call soon such that the callback executes in the
         * thread in which we're running our async I/O loop.
         */
        reactor->call_soon([
            cb = std::move(cb), error = std::move(error),
            answers = std::move(answers)
        ]() { cb(error, answers); });
    });
}

} // namespace dns
} // namespace mk
#endif
