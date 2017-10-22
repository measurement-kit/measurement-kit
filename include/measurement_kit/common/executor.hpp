// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_EXECUTOR_HPP
#define MEASUREMENT_KIT_COMMON_EXECUTOR_HPP

#include <cstddef>                                 // for size_t
#include <measurement_kit/common/callback.hpp>     // for mk::Callback
#include <measurement_kit/common/continuation.hpp> // for mk::Continuation
#include <measurement_kit/common/error.hpp>        // for mk::Error, ...
#include <measurement_kit/common/reactor.hpp>      // for mk::Reactor
#include <measurement_kit/common/shared_ptr.hpp>   // for mk::SharedPtr

namespace mk {

class ExecutorImpl; // Private

class Executor {
  public:
    Executor(SharedPtr<Reactor> reactor, Callback<Error> &&callback);

    Executor &add(Continuation<Error> &&continuation);

    Executor &continue_on_error(bool yesno);

    void start(size_t parallelism);

  private:
    SharedPtr<ExecutorImpl> impl_;
};

} // namespace mk
#endif
