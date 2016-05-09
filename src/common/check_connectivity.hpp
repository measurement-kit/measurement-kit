// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_CHECK_CONNECTIVITY_HPP
#define SRC_COMMON_CHECK_CONNECTIVITY_HPP

#include <measurement_kit/common.hpp>

// Forward declarations
struct event_base;
struct evdns_base;

namespace mk {

/// Check whether the network is up
class CheckConnectivity : public NonCopyable, public NonMovable {

  public:
    /// Returns whether the network is down.
    /// \returns True if the network is down, false otherwise.
    /// \throws std::bad_alloc if some allocation fails.
    /// \throws std::runtime_error if evdns API fails.
    static bool is_down() {
        static CheckConnectivity singleton;
        return !singleton.is_up;
    }

  private:
    event_base *evbase = nullptr;
    evdns_base *dnsbase = nullptr;
    bool is_up = false;

    void cleanup(); // Idempotent cleanup function

    static void dns_callback(int result, char type, int count, int ttl,
                             void *addresses, void *opaque);

    CheckConnectivity();

    ~CheckConnectivity() { cleanup(); }
};

} // namespace mk
#endif
