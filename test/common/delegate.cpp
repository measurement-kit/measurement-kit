// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/delegate.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;

class Helper {
  public:
    void on(std::function<void()> cb) { func_ = cb; }
    void emit() { func_(); }

  private:
    Delegate<> func_;
};

TEST_CASE("Delegate works as expected") {
    Helper helper;
    helper.on([&]() { helper.on([&]() {}); });
    helper.emit();
}
