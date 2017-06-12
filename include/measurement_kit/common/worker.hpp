// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_WORKER_HPP
#define MEASUREMENT_KIT_COMMON_WORKER_HPP

#include <measurement_kit/common/has_global_factory.hpp>
#include <measurement_kit/common/reactor.hpp>

#include <list>
#include <mutex>

namespace mk {

class Worker : public HasGlobalFactory<Worker>,
               public HasMakeFactory<Worker>,
               public NonCopyable,
               public NonMovable {
  public:
    void run_in_background_thread(Callback<> &&func);
    size_t parallelism();
    void set_parallelism(size_t);
    short concurrency();

  private:
    /*
     * Variables allocated indirectly using `Var<T>` because they need to
     * survive this class lifetime when background threads use them.
     */
    Var<std::list<Callback<>>> queue_ = Var<std::list<Callback<>>>::make();
    Var<std::mutex> mutex_ = Var<std::mutex>::make();
    Var<short> active_ = Var<short>::make();

    /*
     * Not allocated using `Var<T>`, because it is only accessed by
     * this object and background threads do not see it.
     */
    size_t parallelism_ = 3;
};

} // namespace mk
#endif
