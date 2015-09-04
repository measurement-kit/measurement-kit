// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_RESOLVER_HPP
#define MEASUREMENT_KIT_DNS_RESOLVER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>

#include <functional>
#include <iosfwd>
#include <string>

struct evdns_base;  // Internally we use evdns

namespace measurement_kit {
namespace dns {

class Response;

using namespace measurement_kit::common;

/*!
 * \brief DNS Resolver object.
 *
 * This object can be used to construct specific DNS resolvers that
 * differ from the default DNS resolver of measurement-kit.
 *
 * In other words, to use the default DNS resolver, one does not need
 * to create an instance of this object.
 *
 * Once a custom resolver is created, one can use it to issue DNS query.
 *
 * For example:
 *
 *     using namespace measurement-kit;
 *
 *     auto reso = dns::Resolver({
 *         {"nameserver", "8.8.8.8"},
 *         {"timeout", "1.0"},
 *         {"attempts", "4"},
 *         {"randomize_case", "1"},
 *     });
 *
 *     reso.query("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
 *             [](dns::Response&& response) {
 *         // Process the response
 *     });
 *
 * The query created this way is bound to the resolver object and is
 * destroyed when the resolver is destroyed (in particular, the callback
 * will be invoked with evdns code equal to DNS_ERR_SHUTDOWN).
 */
class Resolver : public NonCopyable, public NonMovable {

    void cleanup(void);

protected:
    Settings settings;
    Libs *libs = Libs::global();
    Poller *poller = measurement_kit::get_global_poller();
    evdns_base *base = nullptr;
    Logger *logger = Logger::global();

public:
    /*!
     * \brief Default constructor.
     */
    Resolver(void) {
        /* nothing to do */
    }

    /*!
     * \brief Constructor with specific settings.
     * \param settings_ Specific settings. In practice this is a map
     *        from string to string in which the following settings
     *        are accepted:
     *
     *            "nameserver": IP address of nameserver
     *            "attempts": number of query attempts on error
     *            "timeout": timeout in seconds (as a float)
     *            "randomize_case": randomize query's case (0x20 hack)
     *
     *        The default is to use the system's nameserver, to make
     *        3 attempts, to timeout after 5.0 seconds, not to randomize
     *        the case.
     */
    Resolver(Settings settings_, Logger *lp = Logger::global(),
             Libs *lev = nullptr, Poller *plr = nullptr) {
        if (lev != nullptr) {
            libs = lev;
        }
        if (plr != nullptr) {
            poller = plr;
        }
        settings = settings_;
        logger = lp;
    }

    /*!
     * \brief Get the evdns_base bound to the settings.
     * \returns An evdns_base instance.
     * \remark This class uses lazy allocation and the allocation of
     *         the evdns_base occurs the first time you call this method.
     * \throws std::bad_alloc if some allocation fails.
     * \throws std::runtime_error if some edvns API fails.
     */
    evdns_base *get_evdns_base(void);

    /*!
     * \brief Issue a Query using this resolver.
     * \see Query::Query().
     */
    void query(std::string query, std::string address,
               std::function<void(Response&&)>&& func);

    /*!
     * \brief Default destructor.
     */
    ~Resolver(void) {
        cleanup();
    }
};

}}
#endif
