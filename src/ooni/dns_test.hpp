// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_DNS_TEST_HPP
#define MEASUREMENT_KIT_OONI_DNS_TEST_HPP

#include <measurement_kit/common/var.hpp>

#include <measurement_kit/dns.hpp>
#include "src/ooni/net_test.hpp"

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

class DNSTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

    Var<Resolver> resolver;

  public:
    DNSTest(std::string input_filepath_, Settings options_)
        : ooni::NetTest(input_filepath_, options_) {
        test_name = "dns_test";
        test_version = "0.0.1";
    };

    void query(QueryType query_type, QueryClass query_class,
               std::string query_name, std::string nameserver,
               std::function<void(Response)> cb);
};
}
}
#endif
