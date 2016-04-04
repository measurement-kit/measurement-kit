// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_HPP
#define MEASUREMENT_KIT_NDT_NDT_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

void client(std::string address, int port, Callback<> callback,
            Settings settings = {}, Logger *logger = Logger::global(),
            Poller *poller = Poller::global());

} // namespace ndt
} // namespace mk
#endif
