// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/parser_impl.hpp"

namespace mk {
namespace dns {

Error parse_into(Var<Message> message, std::string packet, Var<Logger> logger) {
    return parse_into_impl(message, packet, logger);
}

} // namespace dns
} // namespace mk
