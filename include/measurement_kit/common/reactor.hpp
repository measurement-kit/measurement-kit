// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/socket.hpp>
#include <measurement_kit/common/var.hpp>

// Deprecated since v0.4.x
struct event_base;

#define MK_POLLIN 1 << 0
#define MK_POLLOUT 1 << 1

namespace mk {

class Reactor {
  public:
    static Var<Reactor> make();
    static Var<Reactor> make_remote();
    virtual ~Reactor();

    virtual void call_soon(Callback<> cb) = 0;
    virtual void call_later(double, Callback<> cb) = 0;

    // Deprecated since v0.4.x
    virtual void loop_with_initial_event(Callback<> cb) = 0;
    virtual void loop() = 0;
    virtual void loop_once() = 0;
    virtual void break_loop() = 0;

    /*
        POSIX API for dealing with sockets. Slower than APIs in net,
        especially under Windows, but suitable to integrate with other
        async libraries such as c-ares and perhaps others.
    */
    virtual void pollfd(
                socket_t sockfd,
                short events,
                double timeout,
                Callback<Error, short> callback
        ) = 0;

    void pollfd(socket_t s, short e, Callback<Error, short> f, double t = -1) {
        pollfd(std::move(s), std::move(e), std::move(t), std::move(f));
    }

    // Deprecated since v0.4.x
    virtual event_base *get_event_base() = 0;

    // Introduced as aliases in v0.4.x
    void run_with_initial_event(Callback<> cb) {
        loop_with_initial_event(cb);
    }
    void run() { loop(); }
    void stop() { break_loop(); }

    static Var<Reactor> global();
    static Var<Reactor> global_remote();
};

void call_soon(Callback<>, Var<Reactor> = Reactor::global());
void call_later(double, Callback<>, Var<Reactor> = Reactor::global());
void loop_with_initial_event(Callback<>, Var<Reactor> = Reactor::global());
void loop(Var<Reactor> = Reactor::global());
void loop_once(Var<Reactor> = Reactor::global());
void break_loop(Var<Reactor> = Reactor::global());

// Introduced as aliases in v0.4.x
inline void run_with_initial_event(Callback<> callback,
        Var<Reactor> reactor = Reactor::global()) {
    loop_with_initial_event(callback, reactor);
}
inline void run(Var<Reactor> reactor = Reactor::global()) {
    loop(reactor);
}
inline void stop(Var<Reactor> reactor = Reactor::global()) {
    break_loop(reactor);
}

} // namespace mk
#endif
