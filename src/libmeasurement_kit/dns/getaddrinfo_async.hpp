// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_ASYNC_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_ASYNC_HPP

#include "src/libmeasurement_kit/dns/utils.hpp"
#include "src/libmeasurement_kit/dns/query.hpp"

#include <assert.h>

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
    XX(EAI_SERVICE, NotSupportedServnameError)                                 \
    XX(EAI_SOCKTYPE, NotSupportedAISocktypeError)

inline Error getaddrinfo_async_map_error(int error) {
    switch (error) {
#define XX(code_, class_name_)                                                 \
    case code_:                                                                \
        return class_name_();
        GETADDRINFO_ASYNC_MAP_ERROR
#undef XX
#ifdef EAI_OVERFLOW // Not always available
    case EAI_OVERFLOW:
        return ArgumentBufferOverflowError();
#endif
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
ErrorOr<std::vector<Answer>> getaddrinfo_async_parse_response(
        const std::string &name, addrinfo *rp) {
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
            return {ValueError(), {}}; // Unexpected
        }
        if (p->ai_canonname != nullptr) {
            Answer cname_ans = answer;
            cname_ans.type = MK_DNS_TYPE_CNAME;
            cname_ans.hostname = p->ai_canonname;
            answers.push_back(cname_ans);
            /* FALLTHROUGH */
        }
        char abuf[128];
        if (inet_ntop(p->ai_family, aptr, abuf, sizeof(abuf)) == nullptr) {
            return {GenericError{"inet_ntop_failed"}, {}}; // Unexpected
        }
        if (p->ai_family == AF_INET) {
            answer.ipv4 = abuf;
        } else if (p->ai_family == AF_INET6) {
            answer.ipv6 = abuf;
        } else {
            assert(false); // case excluded above, cannot happen
        }
        answers.push_back(answer);
    }
    return {NoError(), std::move(answers)};
}

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void getaddrinfo_async(std::string name, addrinfo hints, SharedPtr<Reactor> reactor,
                       SharedPtr<Logger> logger,
                       Callback<Error, std::vector<Answer>> cb) {
    /*
     * Move everything down such that there is always just one function in
     * one specific thread having ownership of the state
     */
    reactor->call_in_thread(logger, [
        name = std::move(name), hints = std::move(hints),
        reactor, logger, cb = std::move(cb)
    ]() {
        addrinfo *rp = nullptr;
        Error error = getaddrinfo_async_map_error(
            getaddrinfo(name.c_str(), nullptr, &hints, &rp));
        logger->debug("getaddrinfo('%s') => error: code=%d, reason='%s'",
                      name.c_str(), error.code, error.what());
        std::vector<Answer> answers;
        if (!error && rp != nullptr) {
            ErrorOr<std::vector<Answer>> maybe_answers =
                    getaddrinfo_async_parse_response<inet_ntop>(name, rp);
            if (maybe_answers.as_error() != NoError()) {
                error = maybe_answers.as_error();
            } else {
                std::swap(answers, maybe_answers.as_value());
            }
        }
        if (rp != nullptr) {
            freeaddrinfo(rp);
        }
        reactor->with_current_data_usage([&name, &answers, &logger](
                DataUsage &du) {
            dns::estimate_data_usage(du, name, answers, logger);
        });
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
