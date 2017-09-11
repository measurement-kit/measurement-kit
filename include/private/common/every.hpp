// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_EVERY_HPP
#define PRIVATE_COMMON_EVERY_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <functional>

namespace mk {

static inline void every(const double delay, Var<Reactor> reactor,
                         const std::function<void(Error)> &&callback,
                         const std::function<bool()> &&stop_predicate,
                         const std::function<void()> &&callable) {
    reactor->call_soon([
        delay, reactor, callback = std::move(callback),
        stop_predicate = std::move(stop_predicate),
        callable = std::move(callable)
    ]() {
        if (delay <= 0.0) {
            callback(ValueError());
            return;
        }
        if (stop_predicate()) {
            callback(NoError());
            return;
        }
        callable();
        reactor->call_later(delay, [
            delay, reactor, callback = std::move(callback),
            stop_predicate = std::move(stop_predicate),
            callable = std::move(callable)
        ]() {
            every(delay, reactor, std::move(callback),
                  std::move(stop_predicate), std::move(callable));
        });
    });
}

} // namespace mk
#endif
