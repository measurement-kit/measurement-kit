// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_QUERY_HPP
#define MEASUREMENT_KIT_DNS_QUERY_HPP

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <functional>
#include <iosfwd>

struct evdns_base;  // Internally we use evdns

namespace measurement_kit {
namespace common { struct Libs; }
namespace dns {

class Response;

using namespace measurement_kit::common;

/*!
 * \brief Async DNS request.
 *
 * This is the toplevel class that you should use to issue async
 * DNS requests; it supports A, AAAA and PTR queries.
 *
 * DNS requests issued using directly this class use the default DNS
 * resolver of measurement-kit; use a Resolver object to issue DNS requests
 * that are bound to a specific Resolver.
 *
 * For example:
 *
 *     using namespace measurement-kit;
 *
 *     auto r1 = dns::Query("A", "ooni.torproject.org",
 *             [](dns::Response&& response) {
 *         if (response.get_evdns_code() != DNS_ERR_NONE) {
 *             return;
 *         }
 *         for (auto address : response.get_results()) {
 *             std::cout << address << std::endl;
 *         }
 *     });
 *
 *     auto r2 = dns::Query("REVERSE_AAAA",
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
class Query {

protected:
    SharedPointer<bool> cancelled;

public:

    /*!
     * \brief Default constructor.
     */
    Query() {
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
     * \param libs Optional pointer to a mocked implementation of
     *        the libs object (mainly useful to write unit tests).
     * \throws std::bad_alloc if some allocation fails.
     * \throws std::runtime_error if some edvns API fails.
     */
    Query(std::string query, std::string address,
            std::function<void(Response&&)>&& func,
            Logger *lp = Logger::global(),
            evdns_base *dnsb = nullptr,
            Libs *libs = nullptr);

    Query(Query&) = default;
    Query& operator=(Query&) = default;
    Query(Query&&) = default;
    Query& operator=(Query&&) = default;

    /*!
     * \brief Cancel the pending Query.
     * \remark This method is idempotent.
     */
    void cancel(void);

    /*!
     * \brief Destructor.
     */
    ~Query(void) {
        cancel();
    }
};

}}
#endif
