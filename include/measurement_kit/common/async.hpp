// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ASYNC_HPP
#define MEASUREMENT_KIT_COMMON_ASYNC_HPP

#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/poller.hpp>

namespace measurement_kit {
namespace common {

struct AsyncState;

class Async {
    SharedPointer<AsyncState> state;

    static void loop_thread(SharedPointer<AsyncState>);

  public:

    /// Default constructor
    Async() : Async(SharedPointer<Poller>(new Poller())) {}

    /// Constructor with specified poller
    Async(SharedPointer<Poller> p);

    /// Run the specified network test and call a callback when done
    /// \param func Callback called when test is done
    /// \warn The callback is called from a background thread
    void run_test(SharedPointer<NetTest> test,
      std::function<void(SharedPointer<NetTest>)> func);

    /// Break out of the loop
    void break_loop();

    /// Restart the background loop
    void restart_loop();

    /// Returns true when no async jobs are running
    bool empty();

    ///
    /// Called when the tests queue is empty
    /// \warn This function is called from a background thread
    ///
    void on_empty(std::function<void()>);
};

}}
#endif
