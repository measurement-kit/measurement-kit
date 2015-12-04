// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_POLLER_HPP
#define MEASUREMENT_KIT_COMMON_POLLER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>

struct evdns_base;
struct event_base;

namespace mk {

class Poller : public NonCopyable, public NonMovable {
  public:
    Poller(Libs *libs = nullptr);
    ~Poller();

    event_base *get_event_base() { return base_; }

    evdns_base *get_evdns_base() { return dnsbase_; }

    void loop();

    void loop_once();

    void break_loop();

    static Poller *global() {
        static Poller singleton;
        return &singleton;
    }

  private:
    event_base *base_;
    evdns_base *dnsbase_;
    Libs *libs_ = get_global_libs();
};

/*
 * Syntactic sugar:
 */

inline Poller *get_global_poller(void) {
    return (Poller::global());
}

inline event_base *get_global_event_base(void) {
    return (Poller::global()->get_event_base());
}

inline evdns_base *get_global_evdns_base(void) {
    return (Poller::global()->get_evdns_base());
}

inline void loop(void) { Poller::global()->loop(); }

inline void loop_once(void) { Poller::global()->loop_once(); }

inline void break_loop(void) { Poller::global()->break_loop(); }

} // namespace mk
#endif
