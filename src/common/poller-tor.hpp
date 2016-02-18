// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_TOR_HPP
#define SRC_COMMON_POLLER_TOR_HPP

#include <functional>
#include <list>
#include <measurement_kit/common/var.hpp>
#include "src/common/poller-interface.hpp"
#include <string>

// Forward declarations
struct event_base;
struct evdns_base;

namespace mk {

class PollerTor : public PollerInterface {
  public:
    ~PollerTor() override {}

    event_base *get_event_base() override;

    evdns_base *get_evdns_base() override;

    void call_later(double, std::function<void()>) override;

    void call_soon(std::function<void()>) override;

    void loop() override;

    void loop_once() override;

    void break_loop() override;

    void clear_nameservers() override;

    int count_nameservers() override;

    void add_nameserver(std::string) override;

    static Var<PollerTor> global() {
        static PollerTor singleton;
        return Var<PollerTor>(&singleton, [](PollerTor *){});
    }

  private:
    bool is_running = false;
    bool is_stopped = false;
    std::list<std::function<void()>> early_callbacks;
    evdns_base *dnsbase = nullptr;

    // Private to prevent instantiation
    PollerTor(){}
};

} // namespace
#endif
