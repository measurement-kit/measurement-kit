// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ASYNC_HPP
#define MEASUREMENT_KIT_COMMON_ASYNC_HPP

#include <measurement_kit/common/var.hpp>

#include <functional>
#include <string>

namespace measurement_kit {
namespace common {

class NetTest;
struct AsyncState;

class Async {
  public:
    /// Default constructor
    Async();

    /// Run the specified network test and call a callback when done
    /// \param func Callback called when test is done
    /// \warn The callback is called from a background thread
    void run_test(Var<NetTest> test, std::function<void(Var<NetTest>)> func);

    /// Break out of the loop
    /// \remark This returns immediately, poll empty() to know when
    /// the background thread has terminated.
    void break_loop();

    /// Returns true when no async jobs are running
    bool empty();

    static Async *global(); ///< Returns global async

  private:
    Var<AsyncState> state;
    static void loop_thread(Var<AsyncState>);
};

} // namespace net
} // namespace measurement_kit
#endif
