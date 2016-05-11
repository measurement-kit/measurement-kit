// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_CLIENT_HPP
#define SRC_NDT_CLIENT_HPP

#include "src/ndt/context.hpp"
#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

using Phase = void (*)(Var<Context>, Callback<Error>);
using Cleanup = void (*)(Var<Context>, Error);

} // namespace mk
} // namespace ndt
#endif
