// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_COMMON_ASYNC_RUNNER_HPP
#define PRIVATE_COMMON_ASYNC_RUNNER_HPP

#include <measurement_kit/common/has_global_factory.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/reactor.hpp>

#include <atomic>
#include <cassert>
#include <future>
#include <thread>

namespace mk {

class AsyncRunner : public HasGlobalFactory<AsyncRunner>,
                    public NonCopyable,
                    public NonMovable {
  public:
    // Adapted from src/libmeasurement_kit/nettests/runner.cpp
    template <typename Task, typename Callback>
    void start(std::string &&name, Var<Logger> logger, Task &&task,
               Callback &&cb) {
        assert(active_ >= 0);
        start_background_thread(logger);
        active_ += 1;
        logger->debug("%s: scheduling", name.c_str());
        // Give background thread exclusive ownership of arguments
        reactor_->call_soon([
            task = std::move(task), cb = std::move(cb), name = std::move(name),
            logger, this
        ]() {
            logger->debug("%s: starting", name.c_str());
            // Make sure the lambda's closure keeps the task alive
            task([task, cb, name, logger, this](Error &&error) {
                logger->debug("%s: finished", name.c_str());
                // WARNING: this closure keeps task alive but it MAY be invoked
                // by objects that MAY die after task is destroyed and that also
                // MAY do some work after the closure returns. To avoid issues
                // with use-after-free defer calling the final `cb`, and ensure
                // the closure keeps the task alive until that point in time.
                //
                // Notes:
                //
                // 1. if all objects avoid passing `this` to closures, we
                //    can probably avoid doing this safety step
                //
                // 2. this last callback is mutable so it can be matched
                //    by funtions that take a mutable error as argument
                reactor_->call_soon([task, cb, name, logger, this,
                                     error]() mutable {
                    logger->debug("%s: callback", name.c_str());
                    active_ -= 1;
                    logger->debug("runner: #active: %lld", (long long)active_);
                    cb(std::move(error));
                });
            });
        });
    }

    // Semi-hidden method for now only used internally. The reason why it is
    // not advertised is that it does not provide the guarantees of the above
    // version of `start`, and it must be used with code that does not pass
    // the `this` of objects to any callback.
    template <typename Task>
    void start_(std::string &&name, Var<Logger> logger, Task &&task) {
        start_background_thread(logger);
        assert(active_ >= 0);
        active_ += 1;
        logger->debug("%s: scheduling", name.c_str());
        reactor_->call_soon([
            task = std::move(task), name = std::move(name), logger, this
        ]() mutable {
            logger->debug("%s: starting", name.c_str());
            task([name, logger, this](Callback<> &&cb) mutable {
                logger->debug("%s: finished", name.c_str());
                active_ -= 1;
                logger->debug("runner: #active: %lld", (long long)active_);
                logger->debug("%s: callback", name.c_str());
                cb();
            });
        });
    }

    void stop() {
        if (running_) {
            // WARNING: Make sure we stop the runner before we stop the
            // thread otherwise we will most likely segfault.
            reactor_->stop();
            thread_.join();
            running_ = false;
        }
    }

    long long active() { return active_; }

    bool running() { return running_; }

    ~AsyncRunner() { stop(); }

    Var<Reactor> reactor() { return reactor_; }

  private:
    std::atomic<long long> active_{0};
    Var<Reactor> reactor_ = Reactor::make();
    std::atomic<bool> running_{false};
    std::thread thread_;

    void start_background_thread(Var<Logger> logger) {
        if (!running_) {
            std::promise<void> promise;
            std::future<void> future = promise.get_future();
            // WARNING: the destructor MUST wait for the thread because we
            // are passing `this` to the thread's function.
            logger->debug("runner: starting reactor in background...");
            thread_ = std::thread([&promise, this, logger]() {
                reactor_->run_with_initial_event([&promise, logger]() {
                    logger->debug("runner: background reactor running...");
                    promise.set_value();
                });
            });
            future.wait();
            running_ = true;
        }
    }
};

} // namespace mk
#endif
