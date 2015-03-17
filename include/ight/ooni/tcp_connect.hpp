/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_OONI_TCP_CONNECT_HPP
# define IGHT_OONI_TCP_CONNECT_HPP

#include <ight/ooni/tcp_test.hpp>
#include <sys/stat.h>

namespace ight {
namespace ooni {
namespace tcp_connect {

using namespace ight::common::settings;
using namespace ight::ooni::tcp_test;
using namespace ight::report::entry;

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InputFileRequired : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class TCPConnect : public TCPTest {
    using TCPTest::TCPTest;

    TCPClient client;

    std::function<void(ReportEntry)> have_entry;

public:
    TCPConnect(std::string input_filepath_, Settings options_) : 
      TCPTest(input_filepath_, options_) {
        test_name = "tcp_connect";
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
