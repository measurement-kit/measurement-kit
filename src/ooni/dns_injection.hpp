// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_DNS_INJECTION_HPP
#define MEASUREMENT_KIT_OONI_DNS_INJECTION_HPP

#include <measurement_kit/dns.hpp>
#include "src/ooni/errors.hpp"
#include "src/ooni/net_test.hpp"
#include "src/ooni/dns_test.hpp"
#include <sys/stat.h>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::report;

class DNSInjection : public DNSTest {
    using DNSTest::DNSTest;

    std::function<void(report::Entry)> have_entry;

public:
    DNSInjection(std::string input_filepath_, Settings options_) :
      DNSTest(input_filepath_, options_) {
        test_name = "dns_injection";
        test_version = "0.0.1";

        if (input_filepath_ == "") {
          throw InputFileRequired("An input file is required!");
        }

        struct stat buffer;
        if (stat(input_filepath_.c_str(), &buffer) != 0) {
          throw InputFileDoesNotExist(input_filepath_+" does not exist");
        }
    };

    void main(std::string input, Settings options,
              std::function<void(report::Entry)>&& cb);
};

}}
#endif
