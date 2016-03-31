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
    SafelyOverridableFunc<void()> func_;
};

TEST_CASE("SafelyOverridableFunc works as expected") {
    Helper helper;
    helper.on([&helper]() {
        helper.on([&helper]() {});
    });
    helper.emit();
}

TEST_CASE("AutoResetFunc works as expected") {
    AutoResetFunc<void()> func = []() {};
    func();
    REQUIRE_THROWS(func());
}

TEST_CASE("AutoResetFuncList works as expected") {
    int count = 0;
    AutoResetFuncList<void()> list;
    list += [&count]() { count += 10; };
    list += [&count]() { count += 1; };
    list();
    REQUIRE(count == 11);
}
