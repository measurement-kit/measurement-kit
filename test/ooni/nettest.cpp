#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "ooni/nettest.hpp"
#include "common/log.h"
#include "common/utils.h"

using namespace ight::ooni::nettest;

TEST_CASE("The NetTest should callback when it has finished running") {
  NetTest nettest = NetTest("/tmp/example.txt");
  nettest.begin([&](){
    nettest.end([&](){
      igh_break_loop();
    });
  });
  ight_loop();
}
