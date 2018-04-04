// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_QUERY_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_QUERY_HPP

#include "src/libmeasurement_kit/dns/error.hpp"
#include "src/libmeasurement_kit/dns/query_class.hpp"
#include "src/libmeasurement_kit/dns/query_type.hpp"

#include <measurement_kit/common.hpp>

namespace mk {
namespace dns {

class Answer {
  public:
    QueryType type;
    QueryClass qclass;
    int code = 0;
    uint32_t ttl = 0;
    std::string name;
    std::string ipv4;             ///< For A records
    std::string ipv6;             ///< For AAAA records
    std::string hostname;         ///< For PTR, SOA and CNAME records
    std::string responsible_name; ///< For SOA records
    uint32_t serial_number;       ///< For SOA records
    uint32_t refresh_interval;    ///< For SOA records
    uint32_t retry_interval;      ///< For SOA records
    uint32_t minimum_ttl;         ///< For SOA records
    uint32_t expiration_limit;    ///< For SOA records
};

class Query {
  public:
    QueryType type;
    QueryClass qclass;
    uint32_t ttl = 0;
    std::string name;
};

class Message {
  public:
    Message(){};
    Message(std::nullptr_t){};
    double rtt = 0.0;
    int error_code = 66 /* This is evdns's generic error */;
    std::vector<Answer> answers;
    std::vector<Query> queries;
};

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, SharedPtr<Message>> func,
        Settings settings = {},
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global()
);

} // namespace dns
} // namespace mk
#endif
