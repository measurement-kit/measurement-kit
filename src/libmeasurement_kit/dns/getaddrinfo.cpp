// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/getaddrinfo_impl.hpp"

namespace mk {
namespace dns {

Error eai_to_error(int error) {
    // Adapted from src/libmeasurement_kit/dns/system_resolver.hpp
    switch (error) {
    case EAI_AGAIN:
        return TemporaryFailureError();
    case EAI_BADFLAGS:
        return InvalidFlagsValueError();
#ifdef EAI_BADHINTS // Not always available
    case EAI_BADHINTS:
        return InvalidHintsValueError();
#endif
    case EAI_FAIL:
        return NonRecoverableFailureError();
    case EAI_FAMILY:
        return NotSupportedAIFamilyError();
    case EAI_MEMORY:
        return MemoryAllocationFailureError();
    case EAI_NONAME:
        return HostOrServiceNotProvidedOrNotKnownError();
    case EAI_OVERFLOW:
        return ArgumentBufferOverflowError();
#ifdef EAI_PROTOCOL // Not always available
    case EAI_PROTOCOL:
        return UnknownResolvedProtocolError();
#endif
    case EAI_SERVICE:
        return NotSupportedServnameError();
    case EAI_SOCKTYPE:
        return NotSupportedAISocktypeError();
    default:
        /* Exit the switch to avoid pissing off compilers */
        break;
    }
    return ResolverError();
}

ErrorOr<Var<addrinfo>> getaddrinfo(const char *hostname, const char *port,
                                   addrinfo *hints, Var<Logger> logger) {
    return getaddrinfo_impl(hostname, port, hints, logger);
}

ErrorOr<Var<addrinfo>> getaddrinfo_numeric_datagram(const char *address,
                                                    const char *port,
                                                    Var<Logger> logger) {
    addrinfo hints = {};
    hints.ai_flags |= NI_NUMERICHOST | NI_NUMERICSERV;
    hints.ai_family = SOCK_DGRAM;
    return getaddrinfo_impl(address, port, &hints, logger);
}

ErrorOr<Var<addrinfo>> getaddrinfo_numeric_stream(const char *address,
                                                  const char *port,
                                                  Var<Logger> logger) {
    addrinfo hints = {};
    hints.ai_flags |= NI_NUMERICHOST | NI_NUMERICSERV;
    hints.ai_family = SOCK_STREAM;
    return getaddrinfo_impl(address, port, &hints, logger);
}

} // namespace dns
} // namespace mk
