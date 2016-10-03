// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_SCRIPTABLE_HPP
#define MEASUREMENT_KIT_NDT_SCRIPTABLE_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {
namespace scriptable {

void run(Callback<std::string> callback, Settings settings = {},
         Var<Runner> runner = Runner::global(),
         Var<Logger> logger = Logger::global());

} // namespace scriptable
} // namespace ndt
} // namespace mk
#endif
