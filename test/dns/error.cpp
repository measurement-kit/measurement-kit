// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/dns.hpp>
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

TEST_CASE("Evdns errors are correctly mapped to OONI failures") {

    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_NONE).as_ooni_error() ==
            "");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_FORMAT).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_SERVERFAILED)
                .as_ooni_error() == "dns_lookup_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_NOTEXIST).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_NOTIMPL).as_ooni_error() ==
            "dns_lookup_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_REFUSED).as_ooni_error() ==
            "dns_lookup_error");

    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_TRUNCATED)
                .as_ooni_error() == "dns_lookup_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_UNKNOWN).as_ooni_error() ==
            "unknown_failure 2006");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_TIMEOUT).as_ooni_error() ==
            "generic_timeout_error");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_SHUTDOWN).as_ooni_error() ==
            "unknown_failure 2008");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_CANCEL).as_ooni_error() ==
            "unknown_failure 2009");
    REQUIRE(measurement_kit::dns::dns_error(DNS_ERR_NODATA).as_ooni_error() ==
            "dns_lookup_error");

    // Just three random numbers to increase confidence...
    REQUIRE(measurement_kit::dns::dns_error(1024).as_ooni_error() ==
            "unknown_failure 1");
    REQUIRE(measurement_kit::dns::dns_error(1025).as_ooni_error() ==
            "unknown_failure 1");
    REQUIRE(measurement_kit::dns::dns_error(1026).as_ooni_error() ==
            "unknown_failure 1");
}
