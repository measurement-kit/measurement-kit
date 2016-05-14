// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/dns_injection_impl.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;
using namespace mk::ooni;

TEST_CASE(
    "The DNS Injection test should run with an input file of DNS hostnames") {
    Settings options;
    options["backend"] = "8.8.8.1:53";
    options["dns/timeout"] = 0.1;
    DNSInjectionImpl dns_injection("test/fixtures/hosts.txt", options);
    loop_with_initial_event_and_connectivity([&]() {
        dns_injection.begin(
            [&]() { dns_injection.end([]() { break_loop(); }); });
    });
}

TEST_CASE("The DNS Injection test should throw an exception if an invalid file "
          "path is given") {
    Settings options;
    options["backend"] = "8.8.8.1:53";
    REQUIRE_THROWS_AS(DNSInjectionImpl dns_injection(
                          "/tmp/this-file-does-not-exist.txt", options),
                      InputFileDoesNotExist);
}

TEST_CASE("The DNS Injection test should throw an exception if no file path is "
          "given") {
    Settings options;
    options["backend"] = "8.8.8.1:53";
    REQUIRE_THROWS_AS(DNSInjectionImpl dns_injection("", options),
                      InputFileRequired);
}
