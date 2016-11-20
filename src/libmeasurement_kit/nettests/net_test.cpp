// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

#include <chrono>
#include <ratio>
#include <future>

namespace mk {
namespace nettests {

NetTest &NetTest::on_log(Delegate<uint32_t, const char *> func) {
    logger->on_log(func);
    return *this;
}

NetTest &NetTest::set_verbosity(uint32_t level) {
    logger->set_verbosity(level);
    return *this;
}

NetTest &NetTest::increase_verbosity() {
    logger->increase_verbosity();
    return *this;
}

void NetTest::begin(Callback<Error> func) {
    // You must override this on subclasses to actually start
    // running the test you're interested to run
    reactor->call_soon([=]() { func(NoError()); });
}
void NetTest::end(Callback<Error> func) {
    // You must override this on subclasses to actually terminate
    // running the test (i.e. send results to collector)
    reactor->call_soon([=]() { func(NoError()); });
}

NetTest::NetTest() {}
NetTest::NetTest(Settings o) : options(o) {}
NetTest::NetTest(std::string i, Settings o) : options(o), input_filepath(i) {}
NetTest::~NetTest() {}

NetTest &NetTest::set_input_filepath(std::string s) {
    input_filepath = s;
    return *this;
}

NetTest &NetTest::set_output_filepath(std::string s) {
    output_filepath = s;
    return *this;
}

NetTest &NetTest::set_error_filepath(std::string s) {
    logger->set_logfile(s);
    return *this;
}

NetTest &NetTest::set_reactor(Var<Reactor> r) {
    reactor = r;
    return *this;
}

NetTest &NetTest::on_entry(Delegate<std::string> cb) {
    entry_cb = cb;
    return *this;
}

NetTest &NetTest::on_begin(Delegate<> cb) {
    begin_cb = cb;
    return *this;
}

NetTest &NetTest::on_end(Delegate<> cb) {
    end_cb = cb;
    return *this;
}

Var<NetTest> NetTest::create_test_() {
    // You must override this in subclasses to create the actual
    // instance of the test you would like to run on a runner
    return Var<NetTest>{nullptr};
}

void NetTest::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated reactor, but the code is not yet ready for that.
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    run([&promise]() { promise.set_value(true);});
    future.wait();
}

void NetTest::run(std::function<void()> callback) {
    Runner::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

} // namespace nettests
} // namespace mk
