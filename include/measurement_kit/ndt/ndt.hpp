// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_HPP
#define MEASUREMENT_KIT_NDT_NDT_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

void run_with_specific_server(std::string address, int port,
                              Callback<Error> callback, Settings settings = {},
                              Var<Logger> logger = Logger::global(),
                              Var<Reactor> reactor = Reactor::global());

} // namespace ndt
} // namespace mk
#endif
