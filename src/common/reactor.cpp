// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>
#include "src/common/poller.hpp"

namespace mk {

/*static*/ Var<Reactor> Reactor::make() { return Var<Reactor>(new Poller); }

} // namespace mk
