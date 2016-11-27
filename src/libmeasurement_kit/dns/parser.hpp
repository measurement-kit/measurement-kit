// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_PARSER_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_PARSER_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

/*
 * Design note: I've structured the parser to be independent and to
 * receive as input a standard C++ string, for simplicity. We may
 * want to squeeze more performance by using directly the buffer in
 * which the response was received (this would require to change
 * not only the parser but also how we recv() data) but this is not
 * presently a real concern.
 */

Error parse_into(Var<Message> message, std::string packet,
                 Var<Logger> logger);

} // namespace dns
} // namespace mk
#endif
