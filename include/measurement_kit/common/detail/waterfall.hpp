// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_WATERFALL_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_WATERFALL_HPP

#include <algorithm>                               // for std::move, ...
#include <cstddef>                                 // for size_t
#include <deque>                                   // for std::deque
#include <functional>                              // for std::function
#include <measurement_kit/common/continuation.hpp> // for mk::Continuation
#include <measurement_kit/common/error.hpp>        // for mk::Error, ...
#include <measurement_kit/common/var.hpp>          // for mk::Var, ...
#include <memory>                                  // for std::shared_ptr
#include <mutex>                                   // for std::unique_lock, ...

namespace mk {

class WaterfallExecutor {
  public:
    class Impl {
      public:
        std::deque<Continuation<Error>> continuations;
        std::recursive_mutex mutex;
        std::function<void(Error)> finally;
        Impl(std::function<void(Error)> &&cb) : finally{std::move(cb)} {}
    };

    WaterfallExecutor(std::function<void(Error)> &&callback)
        : impl_{std::make_shared<Impl>(std::move(callback))} {}

    WaterfallExecutor &add(Continuation<Error> &&cc) {
        std::unique_lock<std::recursive_mutex> _{impl_->mutex};
        impl_->continuations.push_back(std::move(cc));
        return *this;
    }

    static void waterfall_next_(Var<Impl> impl) {
        std::unique_lock<std::recursive_mutex> _{impl->mutex};
        if (!impl->continuations.empty()) {
            Continuation<Error> continuation;
            std::swap(continuation, impl->continuations.front());
            impl->continuations.pop_front();
            continuation([impl](Error error) {
                if (error) {
                    impl->finally(error);
                    return;
                }
                waterfall_next_(impl);
            });
        } else {
            impl->finally(NoError());
        }
    }

    void start() {
        std::unique_lock<std::recursive_mutex> _{impl_->mutex};
        waterfall_next_(impl_);
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
