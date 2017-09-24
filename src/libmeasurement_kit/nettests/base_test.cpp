// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/worker.hpp"
#include "private/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>

#include <cassert>
#include <future>

namespace mk {
namespace nettests {

BaseTest &BaseTest::on_logger_eof(Callback<> func) {
    runnable->logger->on_eof(std::move(func));
    return *this;
}

BaseTest &BaseTest::on_log(Callback<uint32_t, const char *> func) {
    runnable->logger->on_log(std::move(func));
    return *this;
}

BaseTest &BaseTest::on_event(Callback<const char *> func) {
    runnable->logger->on_event(std::move(func));
    return *this;
}

BaseTest &BaseTest::on_progress(Callback<double, const char *> func) {
    runnable->logger->on_progress(std::move(func));
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

BaseTest &BaseTest::on_entry(Callback<std::string> cb) {
    runnable->entry_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_begin(Callback<> cb) {
    runnable->begin_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_end(Callback<> cb) {
    runnable->end_cbs.push_back(cb);
    return *this;
}

BaseTest &BaseTest::on_destroy(Callback<> cb) {
    runnable->destroy_cbs.push_back(cb);
    return *this;
}

static void start_internal_(SharedPtr<Runnable> &&r, std::promise<void> *promise,
                            Callback<> &&callback) {
    // Note:
    //
    // 1. we wait in the default tasks queue for our turn: this guarantees
    //    that "big" operations like tests and tasks are sequenced.
    //
    // 2. we move all context inside the first lambda callback so that the
    //    thread that will run I/O has exclusive ownership.
    //
    // 3. run_with_initial_event() is blocking: this means that the lifecycle
    //    of `r` is guaranteed to be as long as the one of the reactor.
    //
    // 4. we _explicitly_ leave the reactor loop on success, but, even in
    //    case the final event is not reached because of a bug, the reactor
    //    is anyway going to exit the loop when it is out of events.
    //
    // 5. the `promise`, if present, allows to make the test synchronous,
    //    while the callback allows to make it asynchronous.
    assert(!r->reactor);
    Worker::default_tasks_queue()->call_in_thread(
          [ r = std::move(r), promise, callback = std::move(callback) ] {
              r->reactor = Reactor::make();
              r->reactor->run_with_initial_event([&]() {
                  r->begin([&](Error) {
                      r->end([&](Error) {
                          r->reactor->stop();
                          if (callback) {
                              callback();
                          }
                      });
                  });
              });
              if (promise != nullptr) {
                  promise->set_value();
              }
          });
}

void BaseTest::run() {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    // Note: using `std::move` to invalidate the `runnable` such that it is
    // not possible to start another test from the same object.
    start_internal_(std::move(runnable), &promise, nullptr);
    future.get();

}

void BaseTest::start(Callback<> callback) {
    // Note: using `std::move` to invalidate the `runnable` such that it is
    // not possible to start another test from the same object.
    start_internal_(std::move(runnable), nullptr, std::move(callback));
}

} // namespace nettests
} // namespace mk
