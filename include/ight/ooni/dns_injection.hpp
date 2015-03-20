/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_OONI_DNS_INJECTION_HPP
# define IGHT_OONI_DNS_INJECTION_HPP

#include <ight/protocols/dns.hpp>
#include <ight/ooni/net_test.hpp>
#include <ight/ooni/dns_test.hpp>
#include <sys/stat.h>

namespace ight {
namespace ooni {
namespace dns_injection {

using namespace ight::common::settings;
using namespace ight::ooni::dns_test;
using namespace ight::report::entry;

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InputFileRequired : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class DNSInjection : public DNSTest {
    using DNSTest::DNSTest;

    std::function<void(ReportEntry)> have_entry;

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
              std::function<void(ReportEntry)>&& cb);
};

}}}
#endif
