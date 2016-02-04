// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/error.hpp>
#include <stdexcept>
#include "src/common/libs_impl.hpp"

// Using `extern "C"` for C callbacks is recommended by C++ FAQs.
// See <https://isocpp.org/wiki/faq/pointers-to-members#memfnptr-vs-fnptr>.
extern "C" {

static void mk_call_soon_cb(evutil_socket_t, short, void *p) {
    auto cbp = static_cast<std::function<void()> *>(p);
    (*cbp)();
    delete cbp;
}

} // extern "C"

namespace mk {

Poller::Poller(Libs *libs) {
    if (libs != nullptr) libs_ = libs;
    if ((base_ = libs_->event_base_new()) == nullptr) throw std::bad_alloc();
    if ((dnsbase_ = libs_->evdns_base_new(base_, 1)) == nullptr) {
        libs_->event_base_free(base_);
        throw std::bad_alloc();
    }
}

Poller::~Poller() {
    libs_->evdns_base_free(dnsbase_, 1);
    libs_->event_base_free(base_);
}

void Poller::call_soon(std::function<void()> cb) {
    auto cbp = new std::function<void()>(cb);
    if (event_base_once(base_, -1, EV_TIMEOUT, mk_call_soon_cb,
                        cbp, nullptr) != 0) {
        delete cbp;
        throw std::runtime_error("event_base_once() failed");
    }
}

void Poller::loop() {
    auto result = libs_->event_base_dispatch(base_);
    if (result < 0) throw std::runtime_error("event_base_dispatch() failed");
    if (result == 1) warn("loop: no pending and/or active events");
}

void Poller::break_loop() {
    if (libs_->event_base_loopbreak(base_) != 0)
        throw std::runtime_error("event_base_loopbreak() failed");
}

void Poller::loop_once() {
    auto result = libs_->event_base_loop(base_, EVLOOP_ONCE);
    if (result < 0) throw std::runtime_error("event_base_loop() failed");
    if (result == 1) warn("loop: no pending and/or active events");
}

void Poller::clear_nameservers() {
    if (evdns_base_clear_nameservers_and_suspend(dnsbase_) != 0 ||
        evdns_base_resume(dnsbase_) != 0) {
        throw std::runtime_error("unexpected evdns failure");
    }
}

int Poller::count_nameservers() {
    return evdns_base_count_nameservers(dnsbase_);
}

void Poller::add_nameserver(std::string address) {
    if (evdns_base_nameserver_ip_add(dnsbase_, address.c_str()) != 0) {
        throw GenericError();
    }
}

} // namespace mk
