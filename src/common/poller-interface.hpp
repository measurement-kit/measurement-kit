// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_INTERFACE_HPP
#define SRC_COMMON_POLLER_INTERFACE_HPP

#include <functional>
#include <string>

// Forward declarations
struct event_base;
struct evdns_base;

namespace mk {

class PollerInterface {
  public:
    virtual ~PollerInterface(){}
    virtual event_base *get_event_base() = 0;
    virtual evdns_base *get_evdns_base() = 0;
    virtual void call_later(double, std::function<void()>) = 0;
    virtual void call_soon(std::function<void()>) = 0;
    virtual void loop() = 0;
    virtual void loop_once() = 0;
    virtual void break_loop() = 0;
    virtual void clear_nameservers() = 0;
    virtual int count_nameservers() = 0;
    virtual void add_nameserver(std::string) = 0;
};

} // namespace
#endif
