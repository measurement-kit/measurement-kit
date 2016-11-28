// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/serializer_impl.hpp"

namespace mk {
namespace dns {

ErrorOr<std::vector<uint8_t>> serialize(std::string name, QueryClass qclass,
                                        QueryType type, unsigned short qid,
                                        int rd, Var<Logger> logger) {
    return serialize_impl(name, qclass, type, qid, rd, logger);
}

} // namespace dns
} // namespace mk
