// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

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
    helper.on([&helper]() {
        helper.on([&helper]() {});
    });
    helper.emit();
}
