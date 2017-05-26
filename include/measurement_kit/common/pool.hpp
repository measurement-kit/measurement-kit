// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_POOL_HPP
#define MEASUREMENT_KIT_COMMON_POOL_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/var.hpp>

#include <set>

namespace mk {

class Pool {
  public:
    Pool();
    ~Pool();

    template <typename T, typename... A> Var<T> alloc(A &&... a) {
        Var<T> p = Var<T>::make(std::forward<A>(a)...);
        active_.insert(p);
        return p;
    }

    template <typename T> void free(Var<T> p) {
        if (dead_.count(p) != 0) {
            throw std::runtime_error("Already dead");
        }
        if (active_.count(p) == 0) {
            throw std::runtime_error("Not found");
        }
        dead_.insert(p);
        active_.erase(p);
    }

    void gc();

  private:
    std::set<Var<void>> active_;
    std::set<Var<void>> dead_;
};

} // namespace mk
#endif
