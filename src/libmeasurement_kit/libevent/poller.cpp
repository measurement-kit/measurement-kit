// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/poller_impl.hpp"

#include <measurement_kit/net/error.hpp>

#include <cassert>

// Using `extern "C"` for C callbacks is recommended by C++ FAQs.
// See <https://isocpp.org/wiki/faq/pointers-to-members#memfnptr-vs-fnptr>.
extern "C" {

void mk_call_later_cb(evutil_socket_t, short, void *p) {
    auto cbp = static_cast<mk::Callback<> *>(p);
    (*cbp)();
    delete cbp;
}

void mk_loop_periodic_cb(evutil_socket_t, short, void *ptr) {
    mk::libevent::Poller *poller = static_cast<mk::libevent::Poller *>(ptr);
    poller->handle_periodic_();
}

void mk_pollfd_cb(evutil_socket_t, short evflags, void *p) {
    auto cbp = static_cast<mk::Callback<mk::Error, short> *>(p);
    mk::Error err = mk::NoError();
    short flags = 0;
    assert((evflags & (EV_TIMEOUT|EV_READ|EV_WRITE)) != 0);
    if ((evflags & EV_TIMEOUT) != 0) {
        err = mk::net::TimeoutError();
    }
    if ((evflags & EV_READ) != 0) {
        flags |= MK_POLLIN;
    }
    if ((evflags & EV_WRITE) != 0) {
        flags |= MK_POLLOUT;
    }
    (*cbp)(err, flags);
    delete cbp;
}

} // extern "C"

namespace mk {
namespace libevent {

Poller::Poller() { base_ = poller_alloc_evbase(); }
Poller::~Poller() {}
event_base *Poller::get_event_base() { return base_.get(); }

void Poller::call_soon(Callback<> &&cb) {
    call_later(0.0, std::move(cb));
}

void Poller::loop_with_initial_event(Callback<> &&cb) {
    call_soon(std::move(cb));
    loop();
}

void Poller::call_later(double timeo, Callback<> &&cb) {
    poller_call_later(base_, timeo, std::move(cb));
}

void Poller::run() { poller_loop(base_, this); }
void Poller::loop_once() { poller_loop_once(base_); }
void Poller::stop() { poller_break_loop(base_); }

void Poller::pollfd(
        socket_t sockfd,
        short events,
        double timeout,
        Callback<Error, short> &&callback) {
    poller_pollfd(base_, sockfd, events, std::move(callback), timeout);
}

void Poller::handle_periodic_() {
    if (periodic_cb_) {
        // Protection against the callback calling on_periodic_()
        // TODO: check whether this is needed (delegate should cope with that)
        auto fn = periodic_cb_;
        fn(this);
    }
}

void Poller::on_periodic_(Callback<Poller *> cb) {
    periodic_cb_ = cb;
}

} // namespace libevent
} // namespace mk
