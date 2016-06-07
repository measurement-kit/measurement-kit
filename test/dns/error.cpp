// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/dns.hpp>
#include <event2/dns.h>

using namespace mk;
using namespace mk::dns;

TEST_CASE("Evdns errors are correctly mapped to OONI failures") {

    REQUIRE(mk::dns::dns_error(DNS_ERR_NONE).as_ooni_error() ==
            "");
    REQUIRE(mk::dns::dns_error(DNS_ERR_FORMAT).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_SERVERFAILED)
                .as_ooni_error() == "dns_lookup_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_NOTEXIST).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_NOTIMPL).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_REFUSED).as_ooni_error() ==
            "dns_lookup_error");

    REQUIRE(mk::dns::dns_error(DNS_ERR_TRUNCATED)
                .as_ooni_error() == "dns_lookup_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_UNKNOWN).as_ooni_error() ==
            "unknown_failure 2006");
    REQUIRE(mk::dns::dns_error(DNS_ERR_TIMEOUT).as_ooni_error() ==
            "generic_timeout_error");
    REQUIRE(mk::dns::dns_error(DNS_ERR_SHUTDOWN).as_ooni_error() ==
            "unknown_failure 2008");
    REQUIRE(mk::dns::dns_error(DNS_ERR_CANCEL).as_ooni_error() ==
            "unknown_failure 2009");
    REQUIRE(mk::dns::dns_error(DNS_ERR_NODATA).as_ooni_error() ==
            "dns_lookup_error");

    // Just three random numbers to increase confidence...
    REQUIRE(mk::dns::dns_error(1024).as_ooni_error() ==
            "unknown_failure 1");
    REQUIRE(mk::dns::dns_error(1025).as_ooni_error() ==
            "unknown_failure 1");
    REQUIRE(mk::dns::dns_error(1026).as_ooni_error() ==
            "unknown_failure 1");
}
