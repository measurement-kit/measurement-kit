// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_WORKER_HPP
#define PRIVATE_COMMON_WORKER_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include <cassert>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace mk {

class Worker {
  public:
    class State : public NonCopyable, public NonMovable {
      public:
        unsigned short active = 0;
        std::mutex mutex;
        unsigned short parallelism = 3;
        std::list<Callback<>> queue;
    };

    void call_in_thread(Callback<> &&func);

    unsigned short parallelism() const;

    void set_parallelism(unsigned short newval) const;

    unsigned short concurrency() const;

    // Implementation note: this method is meant to be used in regress
    // tests, where we don't want the test to exit until the background
    // thread has exited, so to clear thread-local storage. Othrwise,
    // Valgrind will complain about leaked thread-local storage.
    //
    // We expect the caller to issue a blocking command using a Worker
    // and then to call this method such that we keep the main thread
    // alive for longer, so that background threads can exit.
    //
    // Since this is meant for internal-only usage, as explained above,
    // it has been given a name terminating with `_`.
    //
    // See:
    // - test/ooni/orchestrate.cpp
    // - test/nettests/utils.hpp
    void wait_empty_() const;

    static SharedPtr<Worker> default_tasks_queue();

  private:
    SharedPtr<State> state{std::make_shared<State>()};
};

} // namespace mk
#endif
