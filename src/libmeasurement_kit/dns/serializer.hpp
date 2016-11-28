// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SERIALIZER_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SERIALIZER_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

ErrorOr<std::vector<uint8_t>> serialize(std::string name, QueryClass qclass,
                                        QueryType type, unsigned short qid,
                                        int rd, Var<Logger> logger);

} // namespace dns
} // namespace mk
#endif
