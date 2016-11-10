// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_NET_TEST_HPP
#define MEASUREMENT_KIT_COMMON_NET_TEST_HPP

#include <measurement_kit/common/delegate.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/settings.hpp>

namespace mk {

class NetTest {
  public:
    NetTest &on_log(Delegate<uint32_t, const char *> func) {
        logger->on_log(func);
        return *this;
    }
    NetTest &set_verbosity(uint32_t level) {
        logger->set_verbosity(level);
        return *this;
    }
    NetTest &increase_verbosity() {
        logger->increase_verbosity();
        return *this;
    }

    virtual void begin(Callback<Error> func) {
        // You must override this on subclasses to actually start
        // running the test you're interested to run
        reactor->call_soon([=]() { func(NoError()); });
    }
    virtual void end(Callback<Error> func) {
        // You must override this on subclasses to actually terminate
        // running the test (i.e. send results to collector)
        reactor->call_soon([=]() { func(NoError()); });
    }

    NetTest() {}
    NetTest(Settings o) : options(o) {}
    NetTest(std::string i, Settings o) : options(o), input_filepath(i) {}
    virtual ~NetTest();

    NetTest &set_input_filepath(std::string s) {
        input_filepath = s;
        return *this;
    }
    NetTest &set_output_filepath(std::string s) {
        output_filepath = s;
        return *this;
    }
    NetTest &set_error_filepath(std::string s) {
        logger->set_logfile(s);
        return *this;
    }
    NetTest &set_reactor(Var<Reactor> r) {
        reactor = r;
        return *this;
    }
    template <typename T> NetTest &set_options(std::string key, T value) {
        options[key] = value;
        return *this;
    }
    NetTest &on_entry(Delegate<std::string> cb) {
        entry_cb = cb;
        return *this;
    }
    NetTest &on_begin(Delegate<> cb) {
        begin_cb = cb;
        return *this;
    }
    NetTest &on_end(Delegate<> cb) {
        end_cb = cb;
        return *this;
    }

    virtual Var<NetTest> create_test_() {
        // You must override this in subclasses to create the actual
        // instance of the test you would like to run on a runner
        return Var<NetTest>{nullptr};
    }

    // Run in the current thread using the specified `Reactor`, whose event
    // loop is started and terminated when the test is done.
    //
    // Throws an error if the reactor is already running.
    void run(Var<Reactor> r);

    // Run in a background thread using the default `Runner` and block until
    // we reach the end of this test
    void run();

    // Run in the background using the default `Runner` and call the
    // specified callback when test is done
    void run(Callback<> func);

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor = Reactor::global();
    Settings options;
    std::string input_filepath;
    std::string output_filepath;
    Delegate<std::string> entry_cb;
    Delegate<> begin_cb;
    Delegate<> end_cb;
};

} // namespace mk
#endif
