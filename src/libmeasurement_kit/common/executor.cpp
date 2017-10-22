// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <algorithm>                           // for std::move, ...
#include <deque>                               // for std::deque
#include <measurement_kit/common/executor.hpp> // for mk::Executor
#include <memory>                              // for std::shared_ptr
#include <mutex>                               // for std::unique_lock

namespace mk {

class ExecutorImpl {
  public:
    std::deque<Continuation<Error>> continuations;
    std::recursive_mutex mutex;
    Callbacks<Error> finally;
    bool continue_on_error = false;
    bool running = false;
    size_t parallelism = 0;
    size_t completed = 0;
    Error final_error;
    SharedPtr<Reactor> reactor;
    Impl(SharedPtr<Reactor> reactor, Callback<Error> &&cb)
        : finally{reactor, std::move(cb)} {}
};

Executor(SharedPtr<Reactor> reactor, Callback<Error> &&callback)
    : impl_{std::make_shared<ExecutorImpl>(reactor, std::move(callback))} {}

Executor &Executor::add(Continuation<Error> &&cc) {
    std::unique_lock<std::recursive_mutex> _{impl_->mutex};
    if (running) {
        throw std::runtime_error("is running");
    }
    impl_->continuations.push_back(std::move(cc));
    return *this;
}

static void finally_unlocked_(SharedPtr<ExecutorImpl> impl, Error err) {
    impl->final_error.add_child_error(err);
    if (!!err && !impl->final_error) {
        // Set the overall error, but just once
        static const Error error_template = ParallelOperationError();
        impl->final_error.code = error_template.code;
        impl->final_error.reason = error_template.reason;
    }
    impl->completed += 1;
    assert(impl->completed <= impl->parallelism); // Just in case
    if (impl->completed == impl->parallelism) {
        impl->finally(impl->final_error);
    }
}

static void next_(SharedPtr<ExecutorImpl> impl) {
    std::unique_lock<std::recursive_mutex> _{impl->mutex};
    if (!impl->continuations.empty()) {
        Continuation<Error> continuation;
        std::swap(continuation, impl->continuations.front());
        impl->continuations.pop_front();
        continuation([impl](Error error) {
            // Need to lock the mutex because this is most likely to be a
            // deferred callback (just in case, the mutex is recursive)
            std::unique_lock<std::recursive_mutex> _{impl->mutex};
            if (error) {
                finally_unlocked_(impl, error);
                return;
            }
            // Defer `next_()` to defend ourself (and the stack) against the
            // case where continuation do not run deferred operations.
            impl->reactor->call_soon([impl]() { next_(impl); });
        });
    } else {
        finally_unlocked_(impl, NoError());
    }
}

void Executor::start(size_t parallelism) {
    std::unique_lock<std::recursive_mutex> _{impl_->mutex};
    if (parallelism <= 0) {
        throw std::runtime_error("invalid value");
    }
    if (running) {
        throw std::runtime_error("is running");
    }
    impl_->running = true;
    impl_->parallelism = parallelism;
    for (size_t i = 0; i < parallelism; ++i) {
        next_(impl_);
    }
}

} // namespace mk
#endif
