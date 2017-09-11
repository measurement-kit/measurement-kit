// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_LOCKED_HPP
#define PRIVATE_COMMON_LOCKED_HPP

#include <mutex>

namespace mk {

template <typename Mutex, typename Func>
auto locked(Mutex &mutex, Func &&func) {
    std::unique_lock<Mutex> _{mutex};
    return func();
}

template <typename Func> auto locked_global(Func &&func) {
    static std::recursive_mutex mutex;
    return locked(mutex, std::move(func));
}

} // namespace mk
#endif
