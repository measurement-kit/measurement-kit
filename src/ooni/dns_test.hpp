#ifndef LIBIGHT_OONI_DNS_TEST_HPP
# define LIBIGHT_OONI_DNS_TEST_HPP

#include "protocols/dns.hpp"

namespace ight {
namespace ooni {
namespace dns_test {

struct UnsupportedQueryType : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

enum class QueryType {A, NS, MD, MF, CNAME, SOA, MB, MG, MR, NUL, WKS, PTR,
                      HINFO, MINFO, MX, TXT};
enum class QueryClass {IN, CS, CH, HS};

class DNSTest : public nettest::NetTest {

    protocols::dns::Resolver resolver;

public:
    protocols::dns::Request query(QueryType query_type, QueryClass query_class,
        std::string query_name, std::string nameserver,
        std::function<void(protocols::dns::Response&&)>&& cb);

};

}}}

#endif
