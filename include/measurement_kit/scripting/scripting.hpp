// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_SCRIPTING_SCRIPTING_HPP
#define MEASUREMENT_KIT_SCRIPTING_SCRIPTING_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace scripting {

void run(std::string script, Var<Reactor> reactor = Reactor::global(),
         Var<Logger> logger = Logger::global());

} // namespace scripting
} // namespace mk
#endif
