#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/ooni/dns_injection.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

using namespace ight::common::settings;
using namespace ight::ooni::dns_injection;

TEST_CASE("The DNS Injection test should run with an input file of DNS hostnames") {
  ight_set_verbose(1);
  Settings options;
  options["nameserver"] = "8.8.8.8:53";
  DNSInjection dns_injection("test/fixtures/hosts.txt", options);
  dns_injection.begin([&](){
    dns_injection.end([](){
      ight_break_loop();
    });
  });
  ight_loop();
}

TEST_CASE("The DNS Injection test should throw an exception if an invalid file path is given") {
  ight_set_verbose(1);
  Settings options;
  options["nameserver"] = "8.8.8.8:53";
  REQUIRE_THROWS_AS(
      DNSInjection dns_injection("/tmp/this-file-does-not-exist.txt", options),
      InputFileDoesNotExist
  );
}

TEST_CASE("The DNS Injection test should throw an exception if no file path is given") {
  ight_set_verbose(1);
  Settings options;
  options["nameserver"] = "8.8.8.8:53";
  REQUIRE_THROWS_AS(
      DNSInjection dns_injection("", options),
      InputFileRequired
  );
}


