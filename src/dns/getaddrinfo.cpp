// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/inet.h>
#include <event2/dns.h>
#include <event2/thread.h>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/dns/defines.hpp>
#include <measurement_kit/dns/response.hpp>
#include <netdb.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <vector>
#include "src/dns/getaddrinfo.hpp"

namespace mk {

class Logger;

namespace dns {

Maybe<Response> getaddrinfo_query(QueryClass qclass, QueryType qtype,
                                  std::string qname, Logger *logger) {
    logger->debug("getaddrinfo_query qname=%s", qname.c_str());
    // XXX: ignoring qclass and qname
    addrinfo *result = nullptr;
    addrinfo hints;
    memset(&hints, 0, sizeof (hints));
    hints.ai_flags = 0;
    hints.ai_socktype = SOCK_STREAM;
    // TODO: support both IPv4 and IPv6
    hints.ai_family = PF_INET;
    Response response;
    logger->debug("calling getaddrinfo...");
    int code = getaddrinfo(qname.c_str(), nullptr, &hints, &result);
    logger->debug("calling getaddrinfo... done (code=%d)", code);
    if (code != 0) {
        return Maybe<Response>(GenericError(), response);
    }
    char string[128]; // Is wide enough (max. IPv6 length is 45 chars)
    response.type = DNS_IPv4_A;
    addrinfo *ptr = result;
    sockaddr_in *sin;
    for (; ptr != nullptr; ptr = ptr->ai_next) {
        sin = (sockaddr_in *)ptr->ai_addr;
        if (inet_ntop(ptr->ai_family, &sin->sin_addr, string,
                      sizeof (string)) == nullptr) {
            continue;
        }
        response.results.push_back(string);
    }
    response.code = 0;
    return response;
}

} // namespace dns
} // namespace mk
