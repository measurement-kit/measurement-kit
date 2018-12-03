// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_HTTP_HEADER_FIELD_MANIPULATION_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_HTTP_HEADER_FIELD_MANIPULATION_HPP

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

namespace mk {
namespace ooni {

void compare_headers_response(http::Headers headers,
                             SharedPtr<http::Response> response,
                             SharedPtr<nlohmann::json> entry, SharedPtr<Logger> logger);

} // namespace ooni
} // namespace mk

#endif
