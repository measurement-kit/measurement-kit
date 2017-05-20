// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/questions/10626856>
#ifndef MEASUREMENT_KIT_COMMON_FHEAD_HPP
#define MEASUREMENT_KIT_COMMON_FHEAD_HPP

#include <tuple>

namespace mk {

template <typename T, typename... U> T fhead(std::tuple<T, U...> t) {
    return std::get<0>(t);
}

} // namespace mk
#endif
