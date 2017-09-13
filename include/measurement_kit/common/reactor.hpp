// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/socket.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

struct event_base;

namespace mk {

class Reactor {
  public:
    static SharedPtr<Reactor> make();
    static SharedPtr<Reactor> global();
    virtual ~Reactor();

    virtual void run_in_background_thread(Callback<> &&cb) = 0;
    virtual void call_soon(Callback<> &&cb) = 0;
    virtual void call_later(double, Callback<> &&cb) = 0;

    /*
        POSIX API for dealing with sockets. Slower than APIs in net,
        especially under Windows, but suitable to integrate with other
        async libraries such as c-ares and perhaps others.
    */

    virtual void pollin(socket_t sockfd, double timeout,
                        Callback<Error> &&cb) = 0;

    virtual void pollout(socket_t sockfd, double timeout,
                         Callback<Error> &&cb) = 0;

    virtual event_base *get_event_base() = 0;

    void run_with_initial_event(Callback<> &&cb);
    virtual void run() = 0;
    virtual void stop() = 0;
};

} // namespace mk
#endif
