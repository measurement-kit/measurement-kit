// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_LIBEVENT_HPP
#define SRC_COMMON_POLLER_LIBEVENT_HPP

#include <functional>
#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>
#include "src/common/poller-interface.hpp"
#include <string>

// Forward declarations
struct event_base;
struct evdns_base;

namespace mk {

struct Libs; // Forward declaration

class PollerLibevent : public PollerInterface, public NonCopyable,
                       public NonMovable {
  public:
    PollerLibevent(Libs *libs = nullptr);

    ~PollerLibevent() override;

    event_base *get_event_base() override { return base_; }

    evdns_base *get_evdns_base() override { return dnsbase_; }

    void call_later(double, std::function<void()>) override;

    void call_soon(std::function<void()>) override;

    void loop() override;

    void loop_once() override;

    void break_loop() override;

    void clear_nameservers() override;

    int count_nameservers() override;

    void add_nameserver(std::string) override;

  private:
    event_base *base_;
    evdns_base *dnsbase_;
    Libs *libs_ = get_global_libs();
};

} // namespace
#endif
