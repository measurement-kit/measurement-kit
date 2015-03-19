/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_POLLER_HPP
#define IGHT_COMMON_POLLER_HPP

#include <ight/common/constraints.hpp>
#include <ight/common/libevent.hpp>

#include <functional>

namespace ight {
namespace common {
namespace poller {

using namespace ight::common::constraints;
using namespace ight::common::libevent;

class DelayedCall : public NonCopyable, public NonMovable {

    /*
     * A previous implementation of this class required `func` to
     * be a pointer. The current implementation does not. So we can
     * rewrite the code to use an object rather than a pointer.
     */
    std::function<void(void)> *func = NULL;
    event *evp = NULL;
    Libevent *libevent = GlobalLibevent::get();

    // Callback for libevent
    static void dispatch(evutil_socket_t, short, void *);

  public:
    DelayedCall(double, std::function<void(void)> &&, Libevent *libevent = NULL,
                event_base *evbase = NULL);
    ~DelayedCall(void);
};

class Poller : public NonCopyable, public NonMovable {

    event_base *base;
    evdns_base *dnsbase;
    event *evsignal; /* for SIGINT on UNIX */

    Libevent *libevent = GlobalLibevent::get();

  public:
    Poller(Libevent *libevent = NULL);
    ~Poller(void);

    event_base *get_event_base(void) { return (this->base); }

    evdns_base *get_evdns_base(void) { return (this->dnsbase); }

    /*
     * Register a SIGINT handler that breaks the poller loop when
     * the SIGINT signal is received. Generally speaking, a library
     * should provide mechanism, not policy. However, this method
     * is meant to be used in test programs only.
     *
     * The use case for which this functionality exists, in particular,
     * is the following: I want ^C to break the poller loop and lead
     * to a clean exit, so Valgrind can check whether there are leaks
     * for long running test programs (i.e., servers).
     */
    void break_loop_on_sigint_(int enable = 1);

    void loop(void);

    void loop_once(void);

    void break_loop(void);
};

struct GlobalPoller {
    static Poller *get(void) {
        static Poller singleton;
        return (&singleton);
    }
};

}}}

/*
 * Syntactic sugar:
 */

inline ight::common::poller::Poller *ight_get_global_poller(void) {
    return (ight::common::poller::GlobalPoller::get());
}

inline event_base *ight_get_global_event_base(void) {
    return (ight::common::poller::GlobalPoller::get()->get_event_base());
}

inline evdns_base *ight_get_global_evdns_base(void) {
    return (ight::common::poller::GlobalPoller::get()->get_evdns_base());
}

inline void ight_loop(void) {
    ight::common::poller::GlobalPoller::get()->loop();
}

inline void ight_loop_once(void) {
    ight::common::poller::GlobalPoller::get()->loop_once();
}

inline void ight_break_loop(void) {
    ight::common::poller::GlobalPoller::get()->break_loop();
}

#endif
