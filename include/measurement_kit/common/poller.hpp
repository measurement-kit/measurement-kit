// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_POLLER_HPP
#define MEASUREMENT_KIT_COMMON_POLLER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/funcs.hpp>
#include <measurement_kit/common/libs.hpp>

#include <functional>
#include <string>

struct evdns_base;
struct event_base;

namespace mk {

class Poller : public NonCopyable, public NonMovable {
  public:
    Poller(Libs *libs = nullptr);
    ~Poller();

    event_base *get_event_base() { return base_; }

    evdns_base *get_evdns_base() { return dnsbase_; }

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb);

    void call_later(double, std::function<void()> cb);

    void loop_with_initial_event(std::function<void()> cb) {
        call_soon(cb);
        loop();
    }

    void loop();

    // Deprecated: this function was required to implement src/common/async.cpp
    // and after async will be rewritten this function could be removed.
    void loop_once();

    void break_loop();

    // When /etc/resolv.conf is not found, libevent configures as
    // resolver 127.0.0.1:53, which unfortunately is not working on
    // Android. Hence we need the following methods to force the
    // App to clear existing resolver and use another one:

    /// Clear previosuly configured nameservers
    void clear_nameservers();

    /// Get number of configured nameservers in default resolver
    int count_nameservers();

    /// Add nameserver to the list of name-servers of the default resolver
    /// \param address Address and optionally port (e.g. "8.8.8.8:53")
    void add_nameserver(std::string address);

    // End methods to set a different resolver on Android

    static Poller *global() {
        static Poller singleton;
        return &singleton;
    }

    // BEGIN internal functions used to test periodic event functionality
    void handle_periodic_();
    void on_periodic_(std::function<void(Poller *)> callback);
    // END internal functions used to test periodic event functionality

  private:
    event_base *base_;
    evdns_base *dnsbase_;
    Libs *libs_ = get_global_libs();
    SafelyOverridableFunc<void(Poller *)> periodic_cb_;
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

inline void loop_with_initial_event(std::function<void()> cb) {
    Poller::global()->loop_with_initial_event(cb);
}

inline void loop(void) { Poller::global()->loop(); }

inline void loop_once(void) { Poller::global()->loop_once(); }

inline void break_loop(void) { Poller::global()->break_loop(); }

inline void clear_nameservers() { Poller::global()->clear_nameservers(); }

inline int count_nameservers() { return Poller::global()->count_nameservers(); }

inline void add_nameserver(std::string address) {
    Poller::global()->add_nameserver(address);
}

} // namespace mk
#endif
