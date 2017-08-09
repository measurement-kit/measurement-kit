// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/worker.hpp"

#include <measurement_kit/common.hpp>

#include <cassert>
#include <thread>

namespace mk {

size_t Worker::parallelism() {
    return locked(*mutex_, [this]() { return parallelism_; });
}

void Worker::set_parallelism(size_t new_value) {
    locked(*mutex_, [this, &new_value]() {
        parallelism_ = new_value;
    });
}

void Worker::run_in_background_thread(Callback<> &&func) {
    locked(*mutex_, [this, func = std::move(func)]() {

        /*
         * Move function such that the running-in-background thread
         * has unique ownership and controls its lifecycle.
         */
        queue_->push_back(std::move(func));
        /*
         * The following two assertions guarantee that we're not
         * doing mistakes in the future when refactoring.
         */
        assert(*active_ >= 0);
        static_assert(std::is_same<short &, decltype(*active_)>::value,
                      "The type of *active_ has changed");
        if ((unsigned short)*active_ >= parallelism_) { // Cast clearly safe
            return;
        }

        /*
         * Note: we want `mutex`, `queue`, etc. to be safe as long as the
         * thread manipulates them, regardless of `this`'s destiny.
         */
        auto task = [mutex = mutex_, queue = queue_, active = active_]() {
            for (;;) {
                Callback<> func = locked(*mutex, [&queue, &active]() {
                    /*
                     * Initialize inside the lock such that there is only
                     * one critical section in which we could be
                     */
                    if (queue->size() <= 0) {
                        --(*active);
                        return Callback<>{};
                    }
                    auto front = queue->front();
                    queue->pop_front();
                    return front;
                });
                if (!func) {
                    break;
                }
                try {
                    func();
                } catch (const std::exception &exc) {
                    mk::warn("worker thread: unhandled exception: %s",
                             exc.what());
                    /* SUPPRESS */;
                }
            }
        };

        std::thread{task}.detach();
        ++(*active_);
    });
}

short Worker::concurrency() {
    return locked(*mutex_, [this]() { return *active_; });
}

} // namespace mk
