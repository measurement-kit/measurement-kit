// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <measurement_kit/common/poller.hpp>
#include <string>
#include "src/common/poller-libevent.hpp"

namespace mk {

Poller::Poller() : impl(new PollerLibevent){}

Poller::~Poller(){}

event_base *Poller::get_event_base() const { return impl->get_event_base(); }

evdns_base *Poller::get_evdns_base() const { return impl->get_evdns_base(); }

void Poller::call_soon(std::function<void()> cb) const { impl->call_soon(cb); }

void Poller::loop() const { impl->loop(); }

void Poller::loop_once() const { impl->loop_once(); }

void Poller::break_loop() const { impl->break_loop(); }

void Poller::clear_nameservers() const { impl->clear_nameservers(); }

int Poller::count_nameservers() const { return impl->count_nameservers(); }

void Poller::add_nameserver(std::string addr) const {
    impl->add_nameserver(addr);
}

/*static*/ Poller *Poller::global() {
    static Poller singleton;
    return &singleton;
}

} // namespace
