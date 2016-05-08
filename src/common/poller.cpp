// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/poller.hpp"

// Using `extern "C"` for C callbacks is recommended by C++ FAQs.
// See <https://isocpp.org/wiki/faq/pointers-to-members#memfnptr-vs-fnptr>.
extern "C" {

void mk_call_soon_cb(evutil_socket_t, short, void *p) {
    auto cbp = static_cast<std::function<void()> *>(p);
    (*cbp)();
    delete cbp;
}

void mk_do_periodic_cb(evutil_socket_t, short, void *ptr) {
    mk::Poller *poller = static_cast<mk::Poller *>(ptr);
    poller->handle_periodic_();
}

} // extern "C"

namespace mk {

void Poller::call_soon(std::function<void()> cb) {
    call_later(-1.0, cb);
}

void Poller::call_later(double timeo, std::function<void()> cb) {
    call_later_impl(timeo, cb);
}

void Poller::loop() { loop_impl(); }
void Poller::loop_once() { loop_once_impl(); }
void Poller::break_loop() { break_loop_impl(); }

void Poller::handle_periodic_() {
    if (periodic_cb_) {
        // Protection against the callback calling on_periodic_()
        auto fn = periodic_cb_;
        fn(this);
    }
}

void Poller::on_periodic_(std::function<void(Poller *)> cb) {
    periodic_cb_ = cb;
}

} // namespace mk
