#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "ooni/net_test.hpp"
#include "common/poller.h"
#include "common/log.h"
#include "common/utils.hpp"

using namespace ight::ooni::net_test;

TEST_CASE("The NetTest should callback when it has finished running") {
  NetTest net_test("");
  net_test.begin([&](){
    net_test.end([](){
      ight_break_loop();
    });
  });
  ight_loop();
}
