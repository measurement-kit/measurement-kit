#ifndef LIBIGHT_OONI_DNS_TEST_HPP
# define LIBIGHT_OONI_DNS_TEST_HPP

#include "protocols/dns.hpp"
#include "ooni/net_test.hpp"

namespace ight {
namespace ooni {
namespace dns_test {

struct UnsupportedQueryType : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

enum class QueryType {A, NS, MD, MF, CNAME, SOA, MB, MG, MR, NUL, WKS, PTR,
                      HINFO, MINFO, MX, TXT};
enum class QueryClass {IN, CS, CH, HS};

class DNSTest : public net_test::NetTest {
    using net_test::NetTest::NetTest;

    protocols::dns::Resolver resolver;

public:
    DNSTest(std::string input_filepath_, ight::common::Settings options_) : 
      net_test::NetTest(input_filepath_, options_) {
        test_name = "dns_test";
        test_version = "0.0.1";
    };

    void query(QueryType query_type, QueryClass query_class,
        std::string query_name, std::string nameserver,
        std::function<void(protocols::dns::Response&&)>&& cb);

};

}}}

#endif
