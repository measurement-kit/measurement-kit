// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOCKED_HPP
#define MEASUREMENT_KIT_COMMON_LOCKED_HPP

#include <mutex>

namespace mk {

template <typename Func> auto locked(std::mutex &mutex, Func func) {
    std::lock_guard<std::mutex> guard{mutex};
    return func();
}

} // namespace mk
#endif
