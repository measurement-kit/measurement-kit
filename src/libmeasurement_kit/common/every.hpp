// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_EVERY_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_EVERY_HPP

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
