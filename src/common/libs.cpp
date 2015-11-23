// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/libs.hpp>
#include "src/common/libs_impl.hpp"

namespace measurement_kit {

common::Libs *get_global_libs() { return common::Libs::global(); }

} // namespace measurement_kit
