// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/dns_injection.hpp"
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

TEST_CASE(
    "The DNS Injection test should run with an input file of DNS hostnames") {
    measurement_kit::set_verbose(1);
    Settings options;
    options["nameserver"] = "8.8.8.1:53";
    DNSInjection dns_injection("test/fixtures/hosts.txt", options);
    dns_injection.begin(
        [&]() { dns_injection.end([]() { measurement_kit::break_loop(); }); });
    measurement_kit::loop();
}

TEST_CASE("The DNS Injection test should throw an exception if an invalid file "
          "path is given") {
    measurement_kit::set_verbose(1);
    Settings options;
    options["nameserver"] = "8.8.8.1:53";
    REQUIRE_THROWS_AS(DNSInjection dns_injection(
                          "/tmp/this-file-does-not-exist.txt", options),
                      InputFileDoesNotExist);
}

TEST_CASE("The DNS Injection test should throw an exception if no file path is "
          "given") {
    measurement_kit::set_verbose(1);
    Settings options;
    options["nameserver"] = "8.8.8.1:53";
    REQUIRE_THROWS_AS(DNSInjection dns_injection("", options),
                      InputFileRequired);
}
