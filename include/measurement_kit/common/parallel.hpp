// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_PARALLEL_HPP
#define MEASUREMENT_KIT_COMMON_PARALLEL_HPP

#include <measurement_kit/common/continuation.hpp>
#include <measurement_kit/common/error.hpp>

namespace mk {

void parallel(std::vector<Continuation<Error>> input, Callback<Error> cb,
              size_t parallelism = 0 /* means equal to input.size() */);

} // namespace mk
#endif
