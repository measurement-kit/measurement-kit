// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns/response.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/utils.hpp>

#include <event2/dns.h>

#include <cassert>
#include <functional>

#include <sys/socket.h>
#include <limits.h>
#include <stddef.h>

namespace measurement_kit {
namespace dns {

using namespace measurement_kit::common;

Response::Response(int code_, char type_, int count, int ttl_, double started,
                   void *addresses, Logger *logger, Libs *libs, int start_from)
    : code(code_), ttl(ttl_), type(type_) {
    assert(start_from >= 0);

    if (libs == NULL) {
        libs = Libs::global();
    }

    // Only compute RTT when we know that the server replied
    switch (code) {
    case DNS_ERR_NONE:
    case DNS_ERR_FORMAT:
    case DNS_ERR_SERVERFAILED:
    case DNS_ERR_NOTEXIST:
    case DNS_ERR_NOTIMPL:
    case DNS_ERR_REFUSED:
    case DNS_ERR_TRUNCATED:
    case DNS_ERR_NODATA:
        rtt = measurement_kit::time_now() - started;
        break;
    default:
        rtt = 0.0;
        break;
    }

    if (code != DNS_ERR_NONE) {
        logger->info("dns - request failed: %d", code);
        // do not process the results if there was an error

    } else if (type == DNS_PTR) {
        logger->info("dns - PTR");
        // Note: cast magic copied from libevent regress tests
        results.push_back(std::string(*(char **)addresses));

    } else if (type == DNS_IPv4_A || type == DNS_IPv6_AAAA) {

        int family;
        int size;
        char string[128]; // Is wide enough (max. IPv6 length is 45 chars)

        if (type == DNS_IPv4_A) {
            family = PF_INET;
            size = 4;
            logger->info("dns - IPv4");
        } else {
            family = PF_INET6;
            size = 16;
            logger->info("dns - IPv6");
        }

        //
        // Note: make sure in advance `i * size` won't overflow,
        // this is here only for robustness.
        //
        if (count >= 0 && count <= INT_MAX / size + 1) {

            // Note: `start_from`, required by the unit test, defaults to 0
            for (auto i = start_from; i < count; ++i) {
                // Note: address already in network byte order
                if (libs->inet_ntop(family, (char *)addresses + i * size,
                                    string, sizeof(string)) == NULL) {
                    logger->warn("dns - unexpected inet_ntop failure");
                    code = DNS_ERR_UNKNOWN;
                    break;
                }
                logger->info("dns - adding '%s'", string);
                results.push_back(string);
            }

        } else {
            logger->warn("dns - too many addresses");
            code = DNS_ERR_UNKNOWN;
        }

    } else {
        logger->warn("dns - invalid response type");
        code = DNS_ERR_UNKNOWN;
    }
}

std::string Response::map_failure_(int code) {
    std::string s;

    //
    // Here we map evdns error codes to OONI failures.
    //
    // We start with errors specified in RFC 1035 (see also event2/dns.h).
    //
    // TODO for @hellais: make sure that the mapping below is
    // correct with respect to OONI, and eventually consider
    // the possiblity of specifying more errors.
    //

    if (code == DNS_ERR_NONE) {
        // nothing to do

    } else if (code == DNS_ERR_FORMAT) {
        // The name server was unable to interpret the query
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_SERVERFAILED) {
        // The name server was unable to process this query due to a
        // problem with the name server
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_NOTEXIST) {
        // The domain name does not exist
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_NOTIMPL) {
        // The name server does not support the requested kind of query
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_REFUSED) {
        // The name server refuses to perform the specified operation
        // for policy reasons
        s += "dns_lookup_error";

        //
        // The following are evdns specific errors
        //

    } else if (code == DNS_ERR_TRUNCATED) {
        // The reply was truncated or ill-formatted
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_UNKNOWN) {
        // An unknown error occurred
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_TIMEOUT) {
        // Communication with the server timed out
        s += "deferred_timeout_error";

    } else if (code == DNS_ERR_SHUTDOWN) {
        // The request was canceled because the DNS subsystem was shut down.
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_CANCEL) {
        // The request was canceled via a call to evdns_cancel_request
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_NODATA) {
        // There were no answers and no error condition in the DNS packet.
        // This can happen when you ask for an address that exists, but
        // a record type that doesn't.
        s += "dns_lookup_error";

    } else {
        // Safery net - should really not happen
        s += "unknown failure ";
        s += std::to_string(code);
    }

    return s;
}

} // namespace dns
} // namespace measurement_kit
