// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_FUNCS_HPP
#define MEASUREMENT_KIT_COMMON_FUNCS_HPP

#include <cstddef>
#include <functional>
#include <type_traits>
#include <vector>

namespace mk {

template <typename T> class BaseFunc {
  public:
    BaseFunc() {}
    template <typename F> BaseFunc(F f) : func(f) {}
    BaseFunc(std::function<T> f) : func(f) {}

    ~BaseFunc() {}

    void operator=(std::function<T> f) { func = f; }
    template <typename F> void operator=(F f) { func = f; }
    void operator=(std::nullptr_t f) { func = f; }

    // not implementing swap and assign

    operator bool() { return static_cast<bool>(func); }

  protected:
    std::function<T> func;
};

template <typename T> class SafelyOverridableFunc : public BaseFunc<T> {
  public:
    using BaseFunc<T>::BaseFunc;

    template <typename... Args> void operator()(Args &&... args) {
        // Make sure the original closure is not destroyed before end of scope
        auto orig = this->func;
        orig(std::forward<Args>(args)...);
    }
};

template <typename T> class AutoResetFunc : public BaseFunc<T> {
  public:
    using BaseFunc<T>::BaseFunc;

    template <typename... Args> void operator()(Args &&... args) {
        // Automatically reset function before calling it
        auto orig = this->func;
        this->func = nullptr;
        orig(std::forward<Args>(args)...);
    }
};

template <typename T> class AutoResetFuncList {
  public:
    void operator+=(std::function<T> func) {
        funcs_.push_back(func);
    }

    void operator()() {
        for (auto func : funcs_) {
            func();
        }
        funcs_.clear();
    }

  private:
    std::vector<std::function<T>> funcs_;
};

class Error;

template <typename T> using Callback = std::function<void(Error, T)>;

} // namespace
#endif
