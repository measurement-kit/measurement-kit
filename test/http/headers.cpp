// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/http/http.hpp"

using namespace mk;

TEST_CASE("HTTP headers search is case insensitive") {
    http::Headers headers;
    http::headers_push_back(headers, "Location", "https://www.x.org");
    REQUIRE((headers_find_first(headers, "locAtion") == "https://www.x.org"));
}
