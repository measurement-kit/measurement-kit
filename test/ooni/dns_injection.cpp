// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/dns_injection_impl.hpp"
#include <measurement_kit/common.hpp>

using namespace mk::ooni;
using namespace mk::report;
using namespace mk;

TEST_CASE("Check dns-injection output for non-injected entry") {
    loop_with_initial_event([=]() {
        dns_injection(
            "8.8.8.8", "nexa.polito.it", [=](Error err, Var<Entry> entry) {
                REQUIRE(!err);
                std::string temp;
                temp = (*entry)["data_format_version"];
                REQUIRE(temp == "0.2.0.");
                /*
                REQUIRE((*entry)["data_format_version"] == "0.2.0");
                REQUIRE((*entry)["input"] == "nexa.polito.it");
                REQUIRE((*entry)["measurement_start_time"] != nullptr);
                REQUIRE((*entry)["probe_asn"] != nullptr);
                REQUIRE((*entry)["probe_cc"] != nullptr);
                REQUIRE((*entry)["probe_ip"] != nullptr);
                REQUIRE((*entry)["software_name"] == "measurement_kit");
                REQUIRE((*entry)["software_version"] == MEASUREMENT_KIT_VERSION);
                REQUIRE((*entry)["test_name"] == "dns_injection");
                REQUIRE((*entry)["test_runtime"] > 0.0);
                REQUIRE((*entry)["test_start_time"] != nullptr);
                REQUIRE((*entry)["test_version"] != nullptr);
                REQUIRE((*entry)["test_keys"] != nullptr);
                REQUIRE((*entry)["test_keys"]["injected"] == false);
                warn("xx: %s", entry->dump().c_str());
                */
            });
    });
}

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
