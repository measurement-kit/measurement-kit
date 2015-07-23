// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_DNS_TEST_HPP
#define MEASUREMENT_KIT_OONI_DNS_TEST_HPP

#include <measurement_kit/common/pointer.hpp>

#include <measurement_kit/dns/dns.hpp>
#include <measurement_kit/ooni/net_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

struct UnsupportedQueryType : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

enum class QueryType {A, NS, MD, MF, CNAME, SOA, MB, MG, MR, NUL, WKS, PTR,
                      HINFO, MINFO, MX, TXT};
enum class QueryClass {IN, CS, CH, HS};

class DNSTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

    SharedPointer<Resolver> resolver;

public:
    DNSTest(std::string input_filepath_, Settings options_) :
      ooni::NetTest(input_filepath_, options_) {
        test_name = "dns_test";
        test_version = "0.0.1";
    };

    void query(QueryType query_type, QueryClass query_class,
        std::string query_name, std::string nameserver,
        std::function<void(Response&&)>&& cb);

};

}}
#endif
