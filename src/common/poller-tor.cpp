// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/dns.h>
#include <event2/event.h>
#include <event2/util.h>
#include <functional>
#include <mkok-onion-event.h>
#include <measurement_kit/common/error.hpp>
#include <stdexcept>
#include <string>
#include "src/common/poller-tor.hpp"
#include "src/common/utils.hpp"

extern "C" {

static void init_cb(void *opaque) {
    auto func = static_cast<std::function<void()> *>(opaque);
    (*func)();
    delete func;
}

static void event_cb(evutil_socket_t, short, void *opaque) {
    init_cb(opaque);
}

} // extern "C"

namespace mk {

event_base *PollerTor::get_event_base() {
    if (!is_running || is_stopped) {
        throw std::runtime_error("PollerTor: not running or stopped");
    }
    return tor_libevent_get_base();
}

evdns_base *PollerTor::get_evdns_base() {
    if (!is_running || is_stopped) {
        throw std::runtime_error("PollerTor: not running or stopped");
    }
    if (dnsbase == nullptr) {
        dnsbase = ::evdns_base_new(get_event_base(), 1);
        if (dnsbase == nullptr) {
            throw std::runtime_error("PollerTor: evdns_base_new() error");
        }
    }
    return dnsbase;
}

void PollerTor::call_later(double timeout, std::function<void()> cb) {
    if (!is_running) {
        throw std::runtime_error("Is not running");
    }
    if (is_stopped) {
        throw std::runtime_error("PollerTor: is stopped");
    }
    timeval tv, *tvp = timeval_init(&tv, timeout);
    if (event_base_once(get_event_base(), -1, EV_TIMEOUT, event_cb,
                        new std::function<void()>(cb), tvp) != 0) {
        throw std::runtime_error("event_base_once() failed");
    }
}

void PollerTor::call_soon(std::function<void()> cb) {
    if (!is_running) {
        early_callbacks.push_back(cb);
        return;
    }
    call_later(-1, cb);
}

void PollerTor::loop() {
    static const char *argv[] = {
        "tor", "ControlPort", "9051", "DisableNetwork",
        "1",   "ConnLimit",   "50",   nullptr};
    tor_on_started(init_cb, new std::function<void()>([this] {
        is_running = true;
        for (auto &func : early_callbacks) {
            func();
        }
    }));
    try {
        tor_main(7, (char **)argv);
    } catch (...) {
        /* XXX nothing */
        throw;
    }
    is_running = false;
    is_stopped = true;
    // XXX: how do we cleanup the evdns_base?
}

void PollerTor::loop_once() {
    throw std::runtime_error("PollerTor: not implemented");
}

void PollerTor::break_loop() {
    tor_break_loop();
}

void PollerTor::clear_nameservers() {
    if (evdns_base_clear_nameservers_and_suspend(dnsbase) != 0 ||
        evdns_base_resume(dnsbase) != 0) {
        throw std::runtime_error("unexpected evdns failure");
    }
}

int PollerTor::count_nameservers() {
    return evdns_base_count_nameservers(dnsbase);
}

void PollerTor::add_nameserver(std::string address) {
    if (evdns_base_nameserver_ip_add(dnsbase, address.c_str()) != 0) {
        throw GenericError();
    }
}

} // namespace mk
