/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_PROTOCOLS_DNS_HPP
# define LIBIGHT_PROTOCOLS_DNS_HPP

//
// DNS client functionality
//

#include "common/poller.h"

#include <functional>
#include <vector>
#include <string>

struct evdns_base;  // Internally we use evdns

namespace ight {
namespace protocols {
namespace dns {

class RequestImpl;  // Defined in net/dns.cpp

/*!
 * \brief DNS response.
 *
 * The constructor of this class receives the fields returned by evdns and
 * converts them in a format suitable to compile OONI's reports.
 *
 * You should not construct this class directly. Instead, this is the
 * object that Request's callback provides you.
 *
 * \see Request for example usage.
 */
class Response {

    std::string name;
    std::string query_type;
    std::string query_class;
    std::string resolver;
    int code;
    double rtt;
    int ttl;

    std::vector<std::string> results;

public:

    /*!
     * \brief Constructs an empty DNS response object.
     * \remark This is useful to implement the move semantic.
     */
    Response(void);

    /*!
     * \brief Constructs a DNS response object.
     * \param name the requested name (e.g., 'www.google.com').
     * \param query_type the query type (e.g., 'A').
     * \param query_class the query class (e.g., 'IN').
     * \param code evdns status code (e.g., DNS_ERR_NONE -- see
     *        [libevent's github](https://github.com/libevent/libevent/blob/master/include/event2/dns.h#L147) for the available status codes).
     * \param type evdns query type (e.g., DNS_IPv4_A -- see
     *        [libevent's github](https://github.com/libevent/libevent/blob/master/include/event2/dns.h#L176) for the available values).
     * \param count the number of returned records.
     * \param ttl the records time to live.
     * \param started time when the DNS request was started.
     * \param addresses data returned by evdns as an opaque pointer.
     * \param libevent optional pointer to igth's libevent wrapper.
     * \param start_from optional parameter to start from the specified
     *        record, rather than from zero, when processing the results
     *        (this is only used for implementing some test cases).
     */
    Response(std::string name, std::string query_type,
                std::string query_class, std::string resolver,
                int code, char type, int count, int ttl, double started,
                void *addresses, IghtLibevent *libevent = NULL,
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
     * \brief Get the name that was queried (e.g., 'www.google.com').
     */
    std::string get_query_name(void) {
        return name;
    }

    /*!
     * \brief Get the type of the query (e.g., 'A').
     */
    std::string get_query_type(void) {
        return query_type;
    }

    /*!
     * \brief Get the class of the query (e.g., 'IN').
     */
    std::string get_query_class(void) {
        return query_class;
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
     * \brief Get the resolver server address and port.
     * \returns A vector containing two strings, the first string is the
     *          resolver address, the second the resolver port.
     */
    std::vector<std::string> get_resolver(void) {
        if (resolver == "") {
            return {"<default>", "53"};
        }
        auto pos = resolver.find(":");
        if (pos == std::string::npos) {
            return {resolver, "53"};
        }
        return {resolver.substr(0, pos),
            resolver.substr(pos + 1)};
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
    RequestImpl *impl = nullptr;

  public:

    /*!
     * \brief Start an async DNS request.
     * \param query The query type. One of "A", "AAAA", "REVERSE_A" and
     *        "REVERSE_AAAA". Use REVERSE_XXX to issue PTR queries.
     * \param address The address to query for (e.g., "www.neubot.org" for
     *        A and AAAA queries, "82.195.75.101" for REVERSE_A).
     * \param func The callback to call when the response is received; the
     *        callback receives a Response object, make sure you check
     *        the Response status code to see whether there was an error.
     * \param dnsb Optional evdns_base structure to use instead of the
     *        default one. This parameter is not meant to be used directly
     *        by the programmer. To issue requests using a specific evdns_base
     *        with specific options, you should instead use a Resolver.
     * \param resolver Optional address of the DNS nameserver. This address
     *        is not processed by this class, who receives it only for passing
     *        it to the Response constructor. To issue request towards a
     *        specific nameserver, use a Resolver.
     * \param libevent Optional pointer to a mocked implementation of
     *        libight's libevent object (mainly useful to write unit tests).
     * \throws std::bad_alloc if some allocation fails.
     * \throws std::runtime_error if some edvns API fails.
     */
    Request(std::string query, std::string address,
               std::function<void(Response&&)>&& func,
               evdns_base *dnsb = NULL, std::string resolver = "",
               IghtLibevent *libevent = NULL);

    /*!
     * \brief Deleted copy constructor.
     * \remark We cannot copy this object because it contains a pointer
     *         to an object allocated with new that must not be shared.
     */
    Request(Request& /*other*/) = delete;

    /*!
     * \brief Deleted assignment constructor.
     * \remark We cannot copy this object because it contains a pointer
     *         to an object allocated with new that must not be shared.
     */
    Request& operator=(Request& /*other*/) = delete;

    /*!
     * \brief Default move constructor.
     */
    Request(Request&& /*other*/) = default;

    /*!
     * \brief Default move assignment constructor.
     */
    Request& operator=(Request&& /*other*/) = default;

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

class Resolver;  // forward decl.

/*!
 * \brief Contains the settings used by a Resolver.
 *
 * Unless you modify the default configuration, the system wide DNS server
 * is used, every request is retried three times before giving up, the timeout
 * for a request is five seconds (these are the evdns defaults), and the
 * name randomization is *not* used (this is different from evdns defaults).
 *
 * Also, by default the global libight's poller and libevent objects are used.
 *
 * You can change all of this using the setter methods.
 *
 * \see Resolver for example usage.
 */
class Settings {
    friend class Resolver;

    int attempts = -1;
    IghtLibevent *libevent = IghtGlobalLibevent::get();
    std::string nameserver = "";
    IghtPoller *poller = ight_get_global_poller();
    unsigned randomize_case = 0;
    double timeout = -1.0;

public:

    /*!
     * \brief Set number of attempts before a request is considered failed.
     * \param attempts_ Number of attempts before a request is considered
     *        failed (afterwards you get a timeout error).
     * \remark The default value of this parameter is -1 (which means that
     *         evnds default, i.e. 3, must be used).
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_attempts(int attempts_) {
        attempts = attempts_;
        return *this;
    }

    /*!
     * \brief Override the default libight's libevent object.
     * \param libevent_ A mocked libevent object.
     * \remark The global libight's libevent object is used by default.
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_libevent(IghtLibevent *libevent_) {
        // TODO: change this function to receive a IghtLibevent& object
        if (libevent_ != NULL) {
            libevent = libevent_;
        }
        return *this;
    }

    /*!
     * \brief Set the nameserver to be used.
     * \param nameserver_ Nameserver to use expressed by an IP address
     *        followed by an optional port, e.g., '8.8.8.8' or '8.8.8.8:53'.
     * \remark The default value of this parameter is "", meaning that the
     *         evdns code will pick the system's DNS servers (usually
     *         parsing /etc/resolv.conf if you are on Unix).
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_nameserver(std::string nameserver_) {
        nameserver = nameserver_;
        return *this;
    }

    /*!
     * \brief Override the default libight's poller object.
     * \param libevent_ A mocked poller object.
     * \remark The global libight's poller object is used by default.
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_poller(IghtPoller *poller_) {
        // TODO: change this function to receive a IghtPoller& object
        if (poller_ != NULL) {
            poller = poller_;
        }
        return *this;
    }

    /*!
     * \brief Whether to randomize the request's case (a thing also
     *        known as the 0x20 hack and used to mitigate injection attacks).
     * \param randomize_case_ Nonzero to enable this feature, zero to
     *        disable it.
     * \remark The default value of this parameter is 0.
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_randomize_case(unsigned randomize_case_) {
        randomize_case = randomize_case_ ? 1 : 0;  // Normalize value
        return *this;
    }

    /*!
     * \brief Override the default DNS requests timeout.
     * \param timeout_ The new timeout in secionds.
     * \remark The default value of this parameter is -1 (which means that
     *         evdns default, i.e. 5, must be used).
     * \returns A reference to this object, so you can chain calls.
     */
    Settings& set_timeout(double timeout_) {
        timeout = timeout_;
        return *this;
    }

    /*!
     * \brief Default copy constructor.
     * \remark We can safely copy this object because its pointers to
     *         IghtLibevent and IghtPoller are reference-like pointers,
     *         meaning that it's not the responsibility of Settings
     *         to clean up these objects and that Settings assume
     *         that these objects will outlive it. So it is not an issue
     *         to make a copy of such pointers.
     */
    Settings(Settings& /*other*/) = default;

    /*!
     * \brief Default assignment constructor.
     * \remark We can safely copy this object because its pointers to
     *         IghtLibevent and IghtPoller are reference-like pointers,
     *         meaning that it's not the responsibility of Settings
     *         to clean up these objects and that Settings assume
     *         that these objects will outlive it. So it is not an issue
     *         to make a copy of such pointers.
     */
    Settings& operator=(Settings& /*other*/) = default;

    /*!
     * \brief Default move constructor.
     */
    Settings(Settings&& /*other*/) = default;

    /*!
     * \brief Default move assignment.
     */
    Settings& operator=(Settings&& /*other*/) = default;

    /*!
     * \brief Default constructor.
     */
    Settings(void) {
        /* nothing */
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
 *     auto reso = dns::Resolver(dns::Settings()
 *             .set_nameserver("8.8.8.8")
 *             .set_timeout(1.0)
 *             .set_attempts(1));
 *
 *     auto r2 = reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
 *             [](dns::Response&& response) {
 *         // Process the response
 *     });
 */
class Resolver {
    Settings settings;
    evdns_base *base = NULL;

    void cleanup(void);

  public:

    /*!
     * \brief Default constructor.
     */
    Resolver(void) {
        /* nothing to do */
    }

    /*!
     * \brief Constructor with specific settings.
     * \param settings_ Specific settings.
     * \see Settings.
     */
    Resolver(Settings& settings_) {
        settings = settings_;
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
     * \remark This is just a wrapper that calls Request::Request().
     * \see Request::Request().
     */
    Request request(std::string query, std::string address,
                       std::function<void(Response&&)>&& func) {
        return Request(query, address, std::move(func), get_evdns_base(),
                          settings.nameserver, settings.libevent);
    }

    /*!
     * \brief Default destructor.
     */
    ~Resolver(void) {
        cleanup();
    }

    /*!
     * \brief Deleted copy constructor.
     * \remark We cannot copy this object because it contains a pointer
     *         to an object allocated with new that must not be shared.
     */
    Resolver(Resolver& /*other*/) = delete;

    /*!
     * \brief Deleted assignment constructor.
     * \remark We cannot copy this object because it contains a pointer
     *         to an object allocated with new that must not be shared.
     */
    Resolver& operator=(Resolver& /*other*/) = delete;

    /*!
     * \brief Default move constructor.
     */
    Resolver(Resolver&& /*other*/) = default;

    /*!
     * \brief Default move assignment.
     */
    Resolver& operator=(Resolver&& /*other*/) = default;
};

}}}  // namespace
#endif  // LIBIGHT_PROTOCOLS_DNS_HPP
