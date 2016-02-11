// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_FUNC_HPP
#define MEASUREMENT_KIT_COMMON_FUNC_HPP

#include <cstddef>
#include <functional>
#include <type_traits>

namespace mk {

template <typename T> class Func {
  public:
    Func() {}
    Func(std::function<T> f) : func(f) {}

    ~Func() {}

    void operator=(std::function<T> f) { func = f; }
    void operator=(std::nullptr_t f) { func = f; }

    // not implementing swap and assign

    operator bool() { return bool(func); }

    template <typename... Args> void operator()(Args &&... args) {
        // Make sure the original closure is not destroyed before end of scope
        auto backup = func;
        backup(std::forward<Args>(args)...);
    }

  private:
    std::function<T> func;
};

} // namespace
#endif
