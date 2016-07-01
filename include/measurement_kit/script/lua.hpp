// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_SCRIPT_LUA_HPP
#define MEASUREMENT_KIT_SCRIPT_LUA_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace script {

Error run_lua(std::string path) __attribute__((warn_unused_result));

} // namespace script
} // namespace mk
#endif
