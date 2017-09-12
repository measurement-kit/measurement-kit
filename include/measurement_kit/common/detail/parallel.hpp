// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_PARALLEL_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_PARALLEL_HPP

#include <algorithm>                                      // for std::move, ...
#include <cstddef>                                        // for size_t
#include <deque>                                          // for std::deque
#include <functional>                                     // for std::function
#include <measurement_kit/common/detail/continuation.hpp> // for mk::Continuation
#include <measurement_kit/common/error.hpp>               // for mk::Error, ...
#include <measurement_kit/common/var.hpp>                 // for mk::Var, ...
#include <memory>                                         // for std::shared_ptr
#include <mutex>     // for std::unique_lock, ...
#include <stdexcept> // for std::runtime_error

namespace mk {

class ParallelCallback {
  public:
    class Impl {
      public:
        std::function<void(Error)> callback;
        size_t count = 0;
        Error error;
        std::recursive_mutex mutex;
        size_t parallelism;
        Impl(size_t parallelism, const std::function<void(Error)> &&cb)
            : callback{std::move(cb)}, parallelism{parallelism} {}
    };

    ParallelCallback(size_t parallelism, const std::function<void(Error)> &&cb)
        : impl_{mk::make_shared<Impl>(parallelism, std::move(cb))} {}

    void operator()(Error error) const {
        std::unique_lock<std::recursive_mutex> _{impl_->mutex};
        if (++impl_->count > impl_->parallelism) {
            throw std::runtime_error("more callbacks than expected");
        }
        impl_->error.add_child_error(error);
        if (!!error && !impl_->error) {
            const auto poe = ParallelOperationError();
            impl_->error.code = poe.code;
            impl_->error.reason = poe.reason;
        }
        if (impl_->count == impl_->parallelism) {
            impl_->callback(impl_->error);
        }
    }

  private:
    Var<Imp> impl_;
};

class ParallelExecutor {
  public:
    class Impl {
      public:
        std::deque<Continuation<Error>> continuations;
        std::recursive_mutex mutex;
        std::function<void(Error)> finally;
        Impl(std::function<void(Error)> &&cb) : finally{std::move(cb)} {}
    };

    ParallelExecutor(std::function<void(Error)> &&callback)
        : impl_{mk::make_shared<Impl>(std::move(callback))} {}

    ParallelExecutor &add(Continuation<Error> &&cc) {
        std::unique_lock<std::recursive_mutex> _{impl_->mutex};
        impl_->continuations.push_back(std::move(cc));
        return *this;
    }

    static void parallel_next_(Var<Impl> impl, ParallelCallback callback) {
        std::unique_lock<std::recursive_mutex> _{impl->mutex};
        if (!impl->continuations.empty()) {
            Continuation<Error> continuation;
            std::swap(continuation, impl->continuations.front());
            impl->continuations.pop_front();
            continuation([impl, callback](Error error) {
                callback(error);
                parallel_next_(impl, callback);
            });
        }
    }

    void start(size_t parallelism) {
        std::unique_lock<std::recursive_mutex> _{impl_->mutex};
        if (parallelism <= 0) {
            impl_->finally(ValueError());
            return;
        }
        if (impl_->continuations.empty()) {
            impl_->finally(NoError());
            return;
        }
        ParallelCallback callback{impl_->continuations.size(),
                                  std::move(impl_->finally)};
        for (size_t i = 0; i < parallelism; ++i) {
            parallel_next_(impl_, callback);
        }
        // Invalidate Var<pointer> to prevent further usage. Attempting to
        // dereference `impl_` will cause an exception to be thrown.
        //
        // Must be done once we've released the lock. Otherwise it might
        // be that we destroy a locked mutex, which is an exception in libcxx.
        _.unlock();
        impl_.reset();
    }

  private:
    Var<Impl> impl_;
};

} // namespace mk
#endif
