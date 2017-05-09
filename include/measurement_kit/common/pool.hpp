// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_POOL_HPP
#define MEASUREMENT_KIT_COMMON_POOL_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/var.hpp>

#include <map>

namespace mk {

class Pool {
  public:
    Pool();
    ~Pool();

    template <typename T, typename... A>
    Var<T> alloc(std::string s, A &&... a) {
        Var<T> p = mk::make_shared<T>(std::forward<A>(a)...);
        resources_[p] = s;
        return p;
    }

    void query();

  private:
    std::map<Var<void>, std::string> resources_;
};

} // namespace mk
#endif
