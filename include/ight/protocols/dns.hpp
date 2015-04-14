/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_PROTOCOLS_DNS_HPP
# define IGHT_PROTOCOLS_DNS_HPP

//
// DNS client functionality
//

#include <ight/common/constraints.hpp>
#include <ight/common/log.hpp>
#include <ight/common/pointer.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/settings.hpp>

#include <functional>
#include <vector>
#include <string>

struct evdns_base;  // Internally we use evdns

namespace ight {
namespace protocols {
namespace dns {

using namespace ight::common::constraints;
using namespace ight::common::libevent;
using namespace ight::common::log;
using namespace ight::common::pointer;
using namespace ight::common::poller;
using namespace ight::common::settings;

/*!
 * \brief DNS response.
 *
 * You should not construct this class directly. Instead, this is the
 * object that Request's callback provides you.
 *
 * \see Request for example usage.
 */
class Response {

protected:
    int code = 66 /* = DNS_ERR_UNKNOWN */;
    double rtt = 0.0;
    int ttl = 0;

    std::vector<std::string> results;

public:
    Response(Response&) = default;
    Response& operator=(Response&) = default;
    Response(Response&&) = default;
    Response& operator=(Response&&) = default;

    /*!
     * \brief Constructs an empty DNS response object.
     * \remark This is useful to implement the move semantic.
     */
    Response(void) {
        // Nothing to do here
    }

    /*!
     * \brief Constructs a DNS response object.
     * \param code evdns status code (e.g., DNS_ERR_NONE -- see
     *        [libevent's github](https://github.com/libevent/libevent/blob/master/include/event2/dns.h#L147) for the available status codes).
     * \param type evdns query type (e.g., DNS_IPv4_A -- see
     *        [libevent's github](https://github.com/libevent/libevent/blob/master/include/event2/dns.h#L176) for the available values).
     * \param count the number of returned records.
     * \param ttl the records time to live.
     * \param started time when the DNS request was started.
     * \param addresses data returned by evdns as an opaque pointer.
     * \param lp pointer to custom logger object.
     * \param libevent optional pointer to igth's libevent wrapper.
     * \param start_from optional parameter to start from the specified
     *        record, rather than from zero, when processing the results
     *        (this is only used for implementing some test cases).
     */
    Response(int code, char type, int count, int ttl, double started,
             void *addresses,
             SharedPointer<Logger> lp = DefaultLogger::get(),
             Libevent *libevent = NULL,
             int start_from = 0);

    /*!
     * \brief Get the results returned by the query.
     * \returns A list of strings containing the result of the query. Expect
     *          a single entry for PTR queries and one or more entries for
     *          AAAA and A queries.
     */
    std::vector<std::string> get_results(void) {
        return results;
    }

    /*!
     * \brief Get whether the response was authoritative.
     * \bug This method always returns "unknown" since there is no
     *      simple way to get the authoritative flag in evdns.
     */
    std::string get_reply_authoritative(void) {
        return "unknown";  /* TODO */
    }

    /*!
     * \brief Get the integer status code returned by evdns.
     */
    int get_evdns_status(void) {
        return code;
    }

    /*!
     * \brief Map the status code returned by evdns to a OONI failure string.
     */
    std::string get_failure(void) {
        return map_failure_(code);
    }

    /*!
     * \brief Get the time to live of the response.
     */
    int get_ttl(void) {
        return ttl;
    }

    /*!
     * \brief Get the time elapsed since the request was sent until
     *        the response was received.
     * \remark This time is meaningful only when the status code indicates
     *         that evdns received a response from a server. For
     *         example, if the status code indicates that a timeout
     *         expired, no DNS response was obviously received and
     *         the RTT is set to zero.
     */
    double get_rtt(void) {
        return rtt;
    }

    /*!
     * \brief Static function to map evdns status codes to OONI failures.
     * \param code evdns status code.
     * \returns The corresponding OONI failure.
     * \remark This is static and public to ease testing.
     */
    static std::string map_failure_(int code);
};

/*!
 * \brief Async DNS request.
 *
 * This is the toplevel class that you should use to issue async
 * DNS requests; it supports A, AAAA and PTR queries.
 *
 * DNS requests issued using directly this class use the default DNS
 * resolver of libight; use a Resolver object to issue DNS requests
 * that are bound to a specific Resolver.
 *
 * For example:
 *
 *     using namespace ight::protocols;
 *
 *     auto r1 = dns::Request("A", "ooni.torproject.org",
 *             [](dns::Response&& response) {
 *         if (response.get_evdns_code() != DNS_ERR_NONE) {
 *             return;
 *         }
 *         for (auto address : response.get_results()) {
 *             std::cout << address << std::endl;
 *         }
 *     });
 *
 *     auto r2 = dns::Request("REVERSE_AAAA",
 *             "2001:858:2:2:aabb:0:563b:1e28", [](
 *             dns::Response&& response) {
 *         // Process the response
 *     });
 *
 * Note that, for convenience, you don't need to construct the special
 * domain name used for PTR queries. Rather, you only need to pass this
 * class the IPv{4,6} address and the string "REVERSE_FOO" where FOO
 * is "A" for IPv4 and is "AAAA" for IPv6.
 */
class Request {

protected:
    SharedPointer<bool> cancelled;

public:

    /*!
     * \brief Default constructor.
     */
    Request() {
        // nothing to do
    }

    /*!
     * \brief Start an async DNS request.
     * \param query The query type. One of "A", "AAAA", "REVERSE_A" and
     *        "REVERSE_AAAA". Use REVERSE_XXX to issue PTR queries.
     * \param address The address to query for (e.g., "www.neubot.org" for
     *        A and AAAA queries, "82.195.75.101" for REVERSE_A).
     * \param func The callback to call when the response is received; the
     *        callback receives a Response object, make sure you check
     *        the Response status code to see whether there was an error.
     * \param lp Custom logger object.
     * \param dnsb Optional evdns_base structure to use instead of the
     *        default one. This parameter is not meant to be used directly
     *        by the programmer. To issue requests using a specific evdns_base
     *        with specific options, you should instead use a Resolver.
     * \param libevent Optional pointer to a mocked implementation of
     *        libight's libevent object (mainly useful to write unit tests).
     * \throws std::bad_alloc if some allocation fails.
     * \throws std::runtime_error if some edvns API fails.
     */
    Request(std::string query, std::string address,
            std::function<void(Response&&)>&& func,
            SharedPointer<Logger> lp = DefaultLogger::get(),
            evdns_base *dnsb = NULL,
            Libevent *libevent = NULL);

    Request(Request&) = default;
    Request& operator=(Request&) = default;
    Request(Request&&) = default;
    Request& operator=(Request&&) = default;

    /*!
     * \brief Cancel the pending Request.
     * \remark This method is idempotent.
     */
    void cancel(void);

    /*!
     * \brief Destructor.
     */
    ~Request(void) {
        cancel();
    }
};

/*!
 * \brief DNS Resolver object.
 *
 * This object can be used to construct specific DNS resolvers that
 * differ from the default DNS resolver of libight.
 *
 * In other words, to use the default DNS resolver, one does not need
 * to create an instance of this object.
 *
 * Once a custom resolver is created, one can use it to issue DNS requests.
 *
 * For example:
 *
 *     using namespace ight::protocols;
 *
 *     auto reso = dns::Resolver({
 *         {"nameserver", "8.8.8.8"},
 *         {"timeout", "1.0"},
 *         {"attempts", "4"},
 *         {"randomize_case", "1"},
 *     });
 *
 *     reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
 *             [](dns::Response&& response) {
 *         // Process the response
 *     });
 *
 * The request created this way is bound to the resolver object and is
 * destroyed when the resolver is destroyed (in particular, the callback
 * will be invoked with evdns code equal to DNS_ERR_SHUTDOWN).
 */
class Resolver : public NonCopyable, public NonMovable {

    void cleanup(void);

protected:
    Settings settings;
    Libevent *libevent = GlobalLibevent::get();
    Poller *poller = ight_get_global_poller();
    evdns_base *base = NULL;
    SharedPointer<Logger> logger = DefaultLogger::get();

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
     *            "attempts": number of request attempts on error
     *            "timeout": timeout in seconds (as a float)
     *            "randomize_case": randomize query's case (0x20 hack)
     *
     *        The default is to use the system's nameserver, to make
     *        3 attempts, to timeout after 5.0 seconds, not to randomize
     *        the case.
     */
    Resolver(Settings settings_,
             SharedPointer<Logger> lp = DefaultLogger::get(),
             Libevent *lev = NULL, Poller *plr = NULL) {
        if (lev != NULL) {
            libevent = lev;
        }
        if (plr != NULL) {
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
     * \brief Issue a Request using this resolver.
     * \see Request::Request().
     */
    void request(std::string query, std::string address,
                 std::function<void(Response&&)>&& func);

    /*!
     * \brief Default destructor.
     */
    ~Resolver(void) {
        cleanup();
    }
};

}}}
#endif
