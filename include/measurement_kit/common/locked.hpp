// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOCKED_HPP
#define MEASUREMENT_KIT_COMMON_LOCKED_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/return_type.hpp>

#include <mutex>

namespace mk {

template <typename Func, typename = typename std::enable_if<std::is_void<
                             typename ReturnType<Func>::type>::value>::type>
void locked(std::mutex &mutex, Func func) {
    std::lock_guard<std::mutex> guard{mutex};
    func();
}

template <typename Func, typename = typename std::enable_if<!std::is_void<
                             typename ReturnType<Func>::type>::value>::type>
typename ReturnType<Func>::type locked(std::mutex &mutex, Func func) {
    std::lock_guard<std::mutex> guard{mutex};
    return func();
}

} // namespace mk
#endif
