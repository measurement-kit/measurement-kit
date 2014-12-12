#ifndef LIBIGHT_OONI_DNS_INJECTION_HPP
# define LIBIGHT_OONI_DNS_INJECTION_HPP

#include "protocols/dns.hpp"
#include "ooni/net_test.hpp"
#include "ooni/dns_test.hpp"
#include <sys/stat.h>

using namespace ight::ooni::dns_test;

namespace ight {
namespace ooni {
namespace dns_injection {

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InputFileRequired : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class DNSInjection : public DNSTest {
    using DNSTest::DNSTest;

public:
    DNSInjection(std::string input_filepath_, ight::common::Settings options_) : 
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

    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_DNS_INJECTION_HPP
