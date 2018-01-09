// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/questions/10626856>
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_FCAR_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_FCAR_HPP

#include <tuple>

namespace mk {

template <typename T, typename... U>
constexpr T fcar(const std::tuple<T, U...> &t) {
    return std::get<0>(t);
}

} // namespace mk
#endif
