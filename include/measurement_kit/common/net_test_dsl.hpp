// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_NET_TEST_DSL_HPP
#define MEASUREMENT_KIT_COMMON_NET_TEST_DSL_HPP

#include <cstdint>
#include <measurement_kit/common/delegate.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/settings.hpp>
#include <string>

namespace mk {

class NetTest;

/// Internal domain specific language for running network tests. All common
/// test properties are defined explicitly. All other properties are defined
/// implicitly through the generic `set_options()` method.
class NetTestDsl {
  public:
    NetTestDsl() {}
    virtual ~NetTestDsl() {}

    NetTestDsl &set_input_file_path(std::string ifp) {
        input_path = ifp;
        return *this;
    }

    NetTestDsl &set_output_file_path(std::string ofp) {
        output_path = ofp;
        return *this;
    }

    NetTestDsl &set_verbosity(uint32_t level) {
        verbosity = level;
        return *this;
    }

    NetTestDsl &increase_verbosity() {
        verbosity += 1;
        return *this;
    }

    NetTestDsl &set_reactor(Var<Reactor> p) {
        reactor = p;
        return *this;
    }

    template <typename T> NetTestDsl &set_options(std::string key, T value) {
        settings[key] = value;
        return *this;
    }

    NetTestDsl &on_log(Delegate<uint32_t, const char *> func) {
        log_handler = func;
        return *this;
    }

    // This is the method that subclasses must override to define exactly
    // how a network test should be created from these settings
    virtual Var<NetTest> create_test_() { return Var<NetTest>(nullptr); }

    void run();
    void run(std::function<void()> callback);

    Settings settings;
    int verbosity = MK_LOG_WARNING;
    Delegate<uint32_t, const char *> log_handler;
    std::string input_path;
    std::string output_path;
    Var<Reactor> reactor = Reactor::global();
};

#define MK_DECLARE_TEST_DSL(_name_)                                            \
    class _name_ : public mk::NetTestDsl {                                     \
      public:                                                                  \
        Var<NetTest> create_test_() override;                                  \
    };

} // namespace mk
#endif
