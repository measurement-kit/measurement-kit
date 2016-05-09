// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_HPP
#define MEASUREMENT_KIT_NDT_NDT_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

void client(std::string address, int port, Callback<> callback,
            Settings settings = {}, Var<Logger> logger = Logger::global(),
            Var<Reactor> reactor = Reactor::global());

template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

} // namespace ndt
} // namespace mk
#endif
