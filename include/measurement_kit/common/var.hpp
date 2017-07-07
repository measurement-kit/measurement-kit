// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_VAR_HPP
#define MEASUREMENT_KIT_COMMON_VAR_HPP

#include <memory>
#include <stdexcept>

namespace mk {

template <typename T> class Var {
  public:
    template <typename Deleter>
    Var(typename std::add_pointer<T>::type p, Deleter &&deleter)
        : ptr_{std::shared_ptr<T>{p, deleter}} {}

    Var(typename std::add_pointer<T>::type p) : ptr_{std::shared_ptr<T>{p}} {}

    Var(std::shared_ptr<T> &&ptr) : ptr_{ptr} {}

    Var() {}

    long use_count() const noexcept { return ptr_.use_count(); }

    operator bool() const { return static_cast<bool>(ptr_); }

    void reset(typename std::add_pointer<T>::type p = nullptr) {
        ptr_.reset(p);
    }

    bool operator==(std::nullptr_t x) const noexcept {
        return ptr_ == x;
    }

    template <typename Deleter>
    void reset(typename std::add_pointer<T>::type p, Deleter &&deleter) {
        ptr_.reset(p, deleter);
    }

    typename std::add_pointer<T>::type get() const { return operator->(); }

    typename std::add_pointer<T>::type operator->() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.operator->();
    }

    typename std::add_lvalue_reference<T>::type operator*() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.operator*();
    }

    template <typename R> Var<R> as() const {
        return std::dynamic_pointer_cast<R>(ptr_);
    }

    template <typename... A> static Var<T> make(A &&... a) {
        return std::make_shared<T>(std::forward<A>(a)...);
    }

  private:
    std::shared_ptr<T> ptr_;
};

} // namespace mk
#endif
