// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/collector_client_impl.hpp"
#include "src/ooni/dns_injection_impl.hpp"
#include <measurement_kit/common.hpp>

using namespace mk::ooni;
using namespace mk::report;
using namespace mk;

TEST_CASE("Check dns-injection output for injected entry") {
    loop_with_initial_event([=]() {
        dns_injection("8.8.8.8:53", "nexa.polito.it", [=](Error err,
                                                          Var<Entry> entry) {
            // We have called the DNS injection test with an actually
            // valid DNS server, therefore it should return no error and
            // this should be interpreted as DNS injection.
            REQUIRE(!err);
            REQUIRE(entry->is_object());
            bool injected = (*entry)["injected"];
            REQUIRE(injected == true);
            Entry queries = (*entry)["queries"];
            REQUIRE(queries.is_array());
            REQUIRE(queries.size() == 1);
            Entry first_query = queries[0];
            Entry failure = first_query["failure"];
            REQUIRE(failure.is_null());
            std::string hostname = first_query["hostname"];
            REQUIRE(hostname == "nexa.polito.it");
            std::string query_type = first_query["query_type"];
            REQUIRE(query_type == "A");
            std::string resolver_hostname = first_query["resolver_hostname"];
            REQUIRE(resolver_hostname == "8.8.8.8");
            int resolver_port = first_query["resolver_port"];
            REQUIRE(resolver_port == 53);
            Entry answers = first_query["answers"];
            REQUIRE(answers.is_array());
            REQUIRE(answers.size() == 1);
            Entry first_answer = answers[0];
            std::string answer_type = first_answer["answer_type"];
            REQUIRE(answer_type == "A");
            std::string ipv4 = first_answer["ipv4"];
            REQUIRE(ipv4 == "130.192.16.172");
            int ttl = first_answer["ttl"];
            REQUIRE(ttl >= 0);
            break_loop();
        });
    });
}

TEST_CASE("Check dns-injection output for non-injected entry") {
    loop_with_initial_event([=]() {
        dns_injection("8.8.8.1:53", "nexa.polito.it",
                      [=](Error err, Var<Entry> entry) {
                          // We have called the DNS injection test with an
                          // invalid DNS server, therefore it should fail
                          // because of timeout this should be interpreted as
                          // "non injection"
                          REQUIRE(!!err);
                          REQUIRE(entry->is_object());
                          bool injected = (*entry)["injected"];
                          REQUIRE(injected == false);
                          Entry queries = (*entry)["queries"];
                          REQUIRE(queries.is_array());
                          REQUIRE(queries.size() == 1);
                          Entry first_query = queries[0];
                          std::string failure = first_query["failure"];
                          REQUIRE(failure == "generic_timeout_error");
                          std::string hostname = first_query["hostname"];
                          REQUIRE(hostname == "nexa.polito.it");
                          std::string query_type = first_query["query_type"];
                          REQUIRE(query_type == "A");
                          std::string resolver_hostname =
                              first_query["resolver_hostname"];
                          REQUIRE(resolver_hostname == "8.8.8.1");
                          int resolver_port = first_query["resolver_port"];
                          REQUIRE(resolver_port == 53);
                          Entry answers = first_query["answers"];
                          REQUIRE(answers.is_null());
                          break_loop();
                      },
                      // Settings that should make the test much quicker
                      {{"dns/timeout", 0.5}, {"dns/attempts", 1}});
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
