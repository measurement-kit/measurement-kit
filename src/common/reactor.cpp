// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/poller.hpp"
#include <condition_variable>
#include <measurement_kit/common.hpp>
#include <mutex>
#include <thread>

namespace mk {

/*static*/ Var<Reactor> Reactor::make() { return Var<Reactor>(new Poller); }

/*static*/ Var<Reactor> Reactor::make_detached() {
    Var<Reactor> reactor = make();
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    std::condition_variable condition;
    debug("starting reactor in background thread...");
    std::thread thread([&condition, &lock, &mutex, reactor]() {
        reactor->loop_with_initial_event([&condition]() {
            debug("starting reactor in background thread... thread running");
            condition.notify_all();
        });
    });
    condition.wait(lock);
    thread.detach();
    debug("starting reactor in background thread... done");
    return reactor;
}

/*static*/ Var<Reactor> Reactor::global_detached() {
    static Var<Reactor> singleton;
    if (!singleton) {
        // We only create the detached reactor on first request otherwise
        // a thread would always be created even if not used at all
        singleton = make_detached();
    }
    return singleton;
}

} // namespace mk
