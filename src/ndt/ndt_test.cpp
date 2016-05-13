// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {

class NdtTestImpl : public NetTest {
  public:
    using NetTest::NetTest;

    void begin(Callback<> cb) override {
        run([=](Error) { cb(); }, options, logger, reactor);
    }

    void end(Callback<> cb) override {
        cb();
    }
};

Var<NetTest> NdtTest::create_test_() {
    NdtTestImpl *test = new NdtTestImpl(settings);
    test->set_verbosity(verbosity);
    if (log_handler) test->on_log(log_handler);
    test->reactor = reactor;
    return Var<NetTest>(test);
}

} // namespace mk
} // namespace ndt
