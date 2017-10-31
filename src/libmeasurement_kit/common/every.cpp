// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/every.hpp"

namespace mk {

void every(const double delay, SharedPtr<Reactor> reactor,
        Callback<Error> &&callback, std::function<bool()> &&stop_predicate,
        Callback<> &&callable) {
    reactor->call_soon([
        =, callback = std::move(callback),
        stop_predicate = std::move(stop_predicate),
        callable = std::move(callable)
    ]() mutable {
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
            =, callback = std::move(callback),
            stop_predicate = std::move(stop_predicate),
            callable = std::move(callable)
        ]() mutable {
            every(delay, reactor, std::move(callback),
                    std::move(stop_predicate), std::move(callable));
        });
    });
}

} // namespace mk
