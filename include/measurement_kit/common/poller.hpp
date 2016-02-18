// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_POLLER_HPP
#define MEASUREMENT_KIT_COMMON_POLLER_HPP

#include <functional>
#include <measurement_kit/common/var.hpp>
#include <string>

// Forward declarations
struct event_base;
struct evdns_base;

namespace mk {

class PollerInterface; // Forward decl.

class Poller {
  public:
    Poller(Var<PollerInterface> p) : impl(p) {}

    Poller();
    ~Poller();

    event_base *get_event_base() const;

    evdns_base *get_evdns_base() const;

    void call_later(double, std::function<void()> cb) const;

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb) const;

    void loop() const;

    void loop_once() const;

    void break_loop() const;

    // When /etc/resolv.conf is not found, libevent configures as
    // resolver 127.0.0.1:53, which unfortunately is not working on
    // Android. Hence we need the following methods to force the
    // App to clear existing resolver and use another one:

    /// Clear previosuly configured nameservers
    void clear_nameservers() const;

    /// Get number of configured nameservers in default resolver
    int count_nameservers() const;

    /// Add nameserver to the list of name-servers of the default resolver
    /// \param address Address and optionally port (e.g. "8.8.8.8:53")
    void add_nameserver(std::string address) const;

    // End methods to set a different resolver on Android

    static Poller *global();
    static Poller *tor();

  private:
    Var<PollerInterface> impl;
};

/*
 * Syntactic sugar:
 */

inline Poller *get_global_poller() {
    return Poller::global();
}

inline event_base *get_global_event_base() {
    return Poller::global()->get_event_base();
}

inline evdns_base *get_global_evdns_base() {
    return Poller::global()->get_evdns_base();
}

inline void loop() { Poller::global()->loop(); }

inline void loop_once() { Poller::global()->loop_once(); }

inline void break_loop() { Poller::global()->break_loop(); }

inline void clear_nameservers() { Poller::global()->clear_nameservers(); }

inline int count_nameservers() { return Poller::global()->count_nameservers(); }

inline void add_nameserver(std::string address) {
    Poller::global()->add_nameserver(address);
}

} // namespace mk
#endif
