/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_ASYNC_HPP
# define IGHT_COMMON_ASYNC_HPP

#include <ight/common/net_test.hpp>
#include <ight/common/pointer.hpp>
#include <ight/common/poller.hpp>

namespace ight {
namespace common {
namespace async {

using namespace ight::common::net_test;
using namespace ight::common::pointer;
using namespace ight::common::poller;

struct AsyncState;

class Async {
    SharedPointer<AsyncState> state;

    static void loop_thread(SharedPointer<AsyncState>);

  public:

    /// Default constructor
    Async() : Async(SharedPointer<Poller>(new Poller())) {}

    /// Constructor with specified poller
    Async(SharedPointer<Poller> p);

    /// Run the specified network test
    void run_test(SharedPointer<NetTest> test);

    /// Break out of the loop
    void break_loop();

    /// Restart the background loop
    void restart_loop();

    /// Returns true when no async jobs are running
    bool empty();

    ///
    /// Called when a test is completed
    /// \warn This function is called from a background thread
    ///
    void on_complete(std::function<void(SharedPointer<NetTest>)>);

    ///
    /// Called when the tests queue is empty
    /// \warn This function is called from a background thread
    ///
    void on_empty(std::function<void()>);
};

}}}
#endif
