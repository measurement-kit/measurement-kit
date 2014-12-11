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
  options["nameserver"] = "8.8.8.8:52";
  DNSInjection dns_injection("/tmp/foo.txt", options);
  std::cout << "Antani\n";
  dns_injection.begin([&](){
    std::cout << "Antani2\n";
    dns_injection.end([](){
      std::cout << "Antani3\n";
      ight_break_loop();
    });
  });
  ight_loop();
}
