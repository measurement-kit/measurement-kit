// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_EVERY_HPP
#define PRIVATE_COMMON_EVERY_HPP

#include <functional>
#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

namespace mk {

void every(const double delay, SharedPtr<Reactor> reactor,
        Callback<Error> &&callback, std::function<bool()> &&stop_predicate,
        Callback<> &&callable);

} // namespace mk
#endif
