// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/thread.h>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/var.hpp>
#include <stdexcept>
#include "src/common/libs_impl.hpp"
#include "src/common/utils.hpp"

// Using `extern "C"` for C callbacks is recommended by C++ FAQs.
// See <https://isocpp.org/wiki/faq/pointers-to-members#memfnptr-vs-fnptr>.
extern "C" {

static void mk_call_soon_cb(evutil_socket_t, short, void *p) {
    auto cbp = static_cast<std::function<void()> *>(p);
    (*cbp)();
    delete cbp;
}

static void do_periodic(evutil_socket_t, short, void *ptr) {
    mk::Poller *poller = static_cast<mk::Poller *>(ptr);
    poller->handle_periodic_();
}

} // extern "C"

namespace mk {

class EvThreadSingleton {
  private:
    EvThreadSingleton() {
        if (evthread_use_pthreads() != 0) {
            throw std::runtime_error("evthread_use_pthreads() failed");
        }
    }

  public:
    static void ensure() {
        static EvThreadSingleton singleton;
    }
};

Poller::Poller(Libs *libs) {
    if (libs != nullptr) libs_ = libs;
    EvThreadSingleton::ensure();
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

void Poller::call_later(double timeo, std::function<void()> cb) {
    timeval tv, *tvp = timeval_init(&tv, timeo);
    auto cbp = new std::function<void()>(cb);
    if (event_base_once(base_, -1, EV_TIMEOUT, mk_call_soon_cb,
                        cbp, tvp) != 0) {
        delete cbp;
        throw std::runtime_error("event_base_once() failed");
    }
}

void Poller::call_soon(std::function<void()> cb) {
    call_later(-1.0, cb);
}

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

void Poller::loop() {
    // Register a persistent periodic event to make sure that the event
    // loop is not going to exit if we run out of events. This is required
    // to make sure that the ordinary libevent loop works like tor event
    // loop (also based on libevent), which does not exit in any case.
    //
    // Note that the development version of libevent has a flag to implement
    // the behavior described above, but the stable libevent doesn't.
    timeval ten_seconds;
    Var<event> persist(::event_new(base_, -1, EV_PERSIST, do_periodic, this),
                       [](event *p) {
                           if (p != nullptr) {
                               ::event_free(p);
                           }
                       });
    if (!persist) {
        throw std::runtime_error("event_new() failed");
    }
    if (event_add(persist.get(), timeval_init(&ten_seconds, 10.0)) != 0) {
        throw std::runtime_error("event_add() failed");
    }
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
