// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_LOCKED_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_LOCKED_HPP

#include <mutex>

namespace mk {

template <typename Func> auto locked(std::mutex &mutex, Func &&func) {
    std::lock_guard<std::mutex> guard{mutex};
    return func();
}

template <typename Func> auto locked_global(Func &&func) {
    static std::mutex mutex;
    return locked(mutex, std::move(func));
}

} // namespace mk
#endif
