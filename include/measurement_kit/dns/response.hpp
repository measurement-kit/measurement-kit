// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_RESPONSE_HPP
#define MEASUREMENT_KIT_DNS_RESPONSE_HPP

#include <measurement_kit/common/logger.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace measurement_kit {
namespace common { struct Libs; }
namespace dns {

using namespace measurement_kit::common;

/*!
 * \brief DNS response.
 *
 * You should not construct this class directly. Instead, this is the
 * object that Query's callback provides you.
 *
 * \see Query for example usage.
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
     * \param libs optional pointer to igth's libs wrapper.
     * \param start_from optional parameter to start from the specified
     *        record, rather than from zero, when processing the results
     *        (this is only used for implementing some test cases).
     */
    Response(int code, char type, int count, int ttl, double started,
             void *addresses,
             Logger *lp = Logger::global(),
             Libs *libs = nullptr,
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

}}
#endif
