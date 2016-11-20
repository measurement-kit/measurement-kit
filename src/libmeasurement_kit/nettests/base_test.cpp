// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

#include <chrono>
#include <ratio>
#include <future>

namespace mk {
namespace nettests {

BaseTest &BaseTest::on_log(Delegate<uint32_t, const char *> func) {
    runnable->logger->on_log(func);
    return *this;
}

BaseTest &BaseTest::set_verbosity(uint32_t level) {
    runnable->logger->set_verbosity(level);
    return *this;
}

BaseTest &BaseTest::increase_verbosity() {
    runnable->logger->increase_verbosity();
    return *this;
}

BaseTest::BaseTest() {}
BaseTest::~BaseTest() {}

BaseTest &BaseTest::set_input_filepath(std::string s) {
    runnable->input_filepath = s;
    return *this;
}

BaseTest &BaseTest::set_output_filepath(std::string s) {
    runnable->output_filepath = s;
    return *this;
}

BaseTest &BaseTest::set_error_filepath(std::string s) {
    runnable->logger->set_logfile(s);
    return *this;
}

BaseTest &BaseTest::set_reactor(Var<Reactor> r) {
    runnable->reactor = r;
    return *this;
}

BaseTest &BaseTest::on_entry(Delegate<std::string> cb) {
    runnable->entry_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_begin(Delegate<> cb) {
    runnable->begin_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_end(Delegate<> cb) {
    runnable->end_cb = cb;
    return *this;
}

void BaseTest::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated reactor, but the code is not yet ready for that.
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    run([&promise]() { promise.set_value(true);});
    future.wait();
}

void BaseTest::run(Callback<> callback) {
    Runner::global()->run_test(runnable, [=](Var<Runnable>) { callback(); });
}

} // namespace nettests
} // namespace mk
