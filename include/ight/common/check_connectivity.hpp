/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_CHECK_CONNECTIVITY_HPP
# define IGHT_COMMON_CHECK_CONNECTIVITY_HPP

//
// Helper class to check whether we have connectivity
//

#include <ight/common/constraints.hpp>

// Forward declarations
struct event_base;
struct evdns_base;

namespace ight {
namespace common {
namespace check_connectivity {

using namespace ight::common::constraints;

/*!
 * \brief Class used to check whether the network is up.
 *
 * This class is implemented directly on top of evdns, to avoid depending from
 * other libight modules, because it is often used in unit tests. In fact, we
 * have tests that we want to skip when the network is down.
 *
 * To check whether the network is down, do:
 *
 *     if (Network::is_down()) {
 *         return;
 *     }
 *
 * \remark This class reports that the network is up if 8.8.4.4 works and
 * returns a valid IPv4 address for github.com. To reach 8.8.4.4 we use evdns,
 * which we assume to be working. If evdns is broken, 8.8.4.4 is not reachable
 * or github.com is no longer available, this class reports that the network
 * is down even if it is not actually down. All these three conditions are
 * quite unlikely, IMO, so this code should be robust enough.
 *
 * \remark The check on whether the network is down is performed only
 * once when is_down() is called for the first time. Therefore, this
 * class is not suitable to check whether the network is down in long
 * running programs.
 *
 * \warning Because it is designed for unit tests, this class uses its own
 * event loop. This means that, the first time that is_down() is called,
 * the current thread is blocked until the DNS response is received or until
 * the evdns timeout expires.
 */
class Network : public NonCopyable, public NonMovable {
    event_base *evbase = NULL;
    evdns_base *dnsbase = NULL;
    bool is_up = false;

    void cleanup(void);  // Idempotent cleanup function

    static void dns_callback(int result, char type, int count, int ttl,
                             void *addresses, void *opaque);

    Network(void);

    ~Network(void) {
        cleanup();
    }

public:

    /*!
     * \brief Returns whether the network is down.
     * \returns True if the network is down, false otherwise.
     * \throws std::bad_alloc if some allocation fails.
     * \throws std::runtime_error if evdns API fails.
     */
    static bool is_down(void) {
        static Network singleton;
        return !singleton.is_up;
    }
};

}}}
#endif
