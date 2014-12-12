#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "ooni/dns_injection.hpp"
#include "common/poller.h"
#include "common/log.h"
#include "common/utils.h"

using namespace ight::ooni::dns_injection;

TEST_CASE("The DNS Injection test should run") {
  ight_set_verbose(1);
  ight::common::Settings options;
  options["nameserver"] = "8.8.8.8:53";
  DNSInjection dns_injection("test/fixtures/hosts.txt", options);
  dns_injection.begin([&](){
    dns_injection.end([](){
      ight_break_loop();
    });
  });
  ight_loop();
}
