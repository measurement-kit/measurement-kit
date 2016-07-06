// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_LUA_ENGINE_RUN_HPP
#define MEASUREMENT_KIT_LUA_ENGINE_RUN_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace lua_engine {

/// Runs the specified lua script.
/// \param p File path of the script.
/// \throw Error in case of failure.
void run(std::string p);

} // namespace lua_engine
} // namespace mk
#endif
