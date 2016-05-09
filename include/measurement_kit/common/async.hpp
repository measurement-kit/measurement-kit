// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ASYNC_HPP
#define MEASUREMENT_KIT_COMMON_ASYNC_HPP

#include <atomic>
#include <functional>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/var.hpp>
#include <string>
#include <thread>

namespace mk {

class NetTest;

class Async {
  public:
    Async();
    void run_test(Var<NetTest> test, std::function<void(Var<NetTest>)> func);
    void break_loop();
    bool empty();
    void join();
    ~Async();
    static Var<Async> global();

  private:
    std::atomic<int> active{0};
    Var<Reactor> reactor = Reactor::global();
    std::atomic<bool> running{false};
    std::thread thread;
};

} // namespace mk
#endif
