// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_HTTP_HEADER_FIELD_MANIPULATION_HPP
#define PRIVATE_HTTP_HEADER_FIELD_MANIPULATION_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void compare_headers_response(http::Headers headers,
                             SharedPtr<http::Response> response,
                             SharedPtr<report::Entry> entry, SharedPtr<Logger> logger);

} // namespace ooni
} // namespace mk

#endif
