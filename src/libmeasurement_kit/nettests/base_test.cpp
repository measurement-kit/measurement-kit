// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/nettests/runnable.hpp"
#include "private/nettests/runner.hpp"

#include <measurement_kit/nettests.hpp>

#include <cassert>

namespace mk {
namespace nettests {

BaseTest &BaseTest::on_logger_eof(Delegate<> func) {
    runnable->logger->on_eof(func);
    return *this;
}

BaseTest &BaseTest::on_log(Delegate<uint32_t, const char *> func) {
    runnable->logger->on_log(func);
    return *this;
}

BaseTest &BaseTest::on_event(Delegate<const char *> func) {
    runnable->logger->on_event(func);
    return *this;
}

BaseTest &BaseTest::on_progress(Delegate<double, const char *> func) {
    runnable->logger->on_progress(func);
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

BaseTest &BaseTest::add_input(std::string s) {
    // Note: ooni-probe does not allow to specify more than one input from the
    // command line. Given that the underlying code allows that, I do not see a
    // reason to be artifically restrictive here.
    runnable->inputs.push_back(s);
    return *this;
}

BaseTest &BaseTest::add_input_filepath(std::string s) {
    runnable->input_filepaths.push_back(s);
    return *this;
}

BaseTest &BaseTest::set_input_filepath(std::string s) {
    runnable->input_filepaths.clear();
    return add_input_filepath(s);
}

BaseTest &BaseTest::set_output_filepath(std::string s) {
    runnable->output_filepath = s;
    return *this;
}

BaseTest &BaseTest::set_error_filepath(std::string s) {
    runnable->logger->set_logfile(s);
    return *this;
}

BaseTest &BaseTest::set_options(std::string key, std::string value) {
    runnable->options[key] = value;
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
    runnable->end_cbs.push_back(cb);
    return *this;
}

BaseTest &BaseTest::on_destroy(Delegate<> cb) {
    runnable->destroy_cbs.push_back(cb);
    return *this;
}

void BaseTest::run() {
    // Note: here we MUST point to a fresh reactor which we know for sure is
    // not already being used otherwise we cannot run the test
    assert(not runnable->reactor);
    Var<Reactor> reactor = Reactor::make();
    runnable->reactor = reactor;
    reactor->loop_with_initial_event([&]() {
        runnable->begin([&](Error) {
            runnable->end([&](Error) {
                reactor->call_soon([&]() {
                    reactor->stop();
                });
            });
        });
    });
}

void BaseTest::start(Callback<> callback) {
    // Important: here we must move the runnable inside start_test() to avoid
    // a reference loop that prevents node bindings from leaving the loop.
    Runner::global()->start_test(std::move(runnable),
                                 [=](Var<Runnable>) { callback(); });
}

} // namespace nettests
} // namespace mk
