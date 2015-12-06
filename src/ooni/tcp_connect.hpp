// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_CONNECT_HPP
#define MEASUREMENT_KIT_OONI_TCP_CONNECT_HPP

#include "src/ooni/errors.hpp"
#include "src/ooni/tcp_test.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

class TCPConnect : public TCPTest {
    using TCPTest::TCPTest;

    TCPClient client;

    std::function<void(report::Entry)> have_entry;

  public:
    TCPConnect(std::string input_filepath_, Settings options_)
        : TCPTest(input_filepath_, options_) {
        test_name = "tcp_connect";
        test_version = "0.0.1";

        if (input_filepath_ == "") {
            throw InputFileRequired("An input file is required!");
        }

        struct stat buffer;
        if (stat(input_filepath_.c_str(), &buffer) != 0) {
            throw InputFileDoesNotExist(input_filepath_ + " does not exist");
        }
    };

    void main(std::string input, Settings options,
              std::function<void(report::Entry)> &&cb);
};

} // namespace ooni
} // namespace mk
#endif
