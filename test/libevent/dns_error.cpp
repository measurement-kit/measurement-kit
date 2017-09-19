// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/dns.hpp>
#include <event2/dns.h>

#include "private/libevent/dns.hpp"

using namespace mk;
using namespace mk::libevent;

TEST_CASE("Evdns errors are correctly mapped to OONI failures") {

    REQUIRE(dns_error(DNS_ERR_NONE).reason ==
            "");
    REQUIRE(dns_error(DNS_ERR_FORMAT).reason ==
            "dns_lookup_error");
    REQUIRE(dns_error(DNS_ERR_SERVERFAILED)
                .reason == "dns_lookup_error");
    REQUIRE(dns_error(DNS_ERR_NOTEXIST).reason ==
            "dns_lookup_error");
    REQUIRE(dns_error(DNS_ERR_NOTIMPL).reason ==
            "dns_lookup_error");
    REQUIRE(dns_error(DNS_ERR_REFUSED).reason ==
            "dns_lookup_error");

    REQUIRE(dns_error(DNS_ERR_TRUNCATED)
                .reason == "dns_lookup_error");
    REQUIRE(dns_error(DNS_ERR_UNKNOWN).reason ==
            "dns_unknown_error");
    REQUIRE(dns_error(DNS_ERR_TIMEOUT).reason ==
            "generic_timeout_error");
    REQUIRE(dns_error(DNS_ERR_SHUTDOWN).reason ==
            "dns_shutdown");
    REQUIRE(dns_error(DNS_ERR_CANCEL).reason ==
            "dns_cancel");
    REQUIRE(dns_error(DNS_ERR_NODATA).reason ==
            "dns_lookup_error");

    // Just three random numbers to increase confidence...
    REQUIRE(dns_error(1024).reason ==
            "generic_error");
    REQUIRE(dns_error(1025).reason ==
            "generic_error");
    REQUIRE(dns_error(1026).reason ==
            "generic_error");
}
