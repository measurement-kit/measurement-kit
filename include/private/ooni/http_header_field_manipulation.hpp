// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_HTTP_HEADER_FIELD_MANIPULATION_HPP
#define PRIVATE_HTTP_HEADER_FIELD_MANIPULATION_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void compare_headers_response(http::Headers headers,
                             Var<http::Response> response,
                             Var<report::Entry> entry, Var<Logger> logger);

} // namespace ooni
} // namespace mk

#endif
