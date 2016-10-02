// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_RUNNER_HPP
#define MEASUREMENT_KIT_COMMON_RUNNER_HPP

#include <measurement_kit/common/reactor.hpp>

#include <atomic>
#include <string>
#include <thread>

namespace mk {

class NetTest;

class Runner {
  public:
    Runner();
    void run_test(Var<NetTest> test, std::function<void(Var<NetTest>)> func);
    void run(Callback<Continuation<>> begin);
    void break_loop();
    bool empty();
    void join();
    ~Runner();
    static Var<Runner> global();

    // Globally accessible attribute that other classes can use
    Var<Reactor> reactor = Reactor::global();

  private:
    std::atomic<int> active{0};
    std::mutex run_mutex;
    std::atomic<bool> running{false};
    std::thread thread;
};

} // namespace mk
#endif
