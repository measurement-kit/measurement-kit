// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SERIALIZER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SERIALIZER_IMPL_HPP

#include <cassert>

#include <ares.h>

#include "../dns/ares_map_failure.hpp"
#include "../dns/serializer.hpp"

namespace mk {
namespace dns {

template <MK_MOCK(ares_create_query), MK_MOCK(ares_free_string)>
ErrorOr<std::vector<uint8_t>>
serialize_impl(std::string name, QueryClass qclass, QueryType type,
               unsigned short qid, int rd, Var<Logger> logger) {
    uint8_t *buf = nullptr;
    int buflen = 0;
    int status = ares_create_query(name.c_str(), qclass, type, qid, rd, &buf,
                                   &buflen, 0);
    if (status != ARES_SUCCESS) {
        Error err = ares_map_failure(status);
        logger->warn("dns: ares_create_query() failed: %s",
                     err.explain().c_str());
        return err;
    }
    assert(buf != nullptr && buflen > 0);
    // Cast checks here guaranteed by the above assert statement
    std::vector<uint8_t> rv{buf, buf + (size_t)buflen};
    ares_free_string(buf);
    return rv;
}

} // namespace dns
} // namespace mk
#endif
