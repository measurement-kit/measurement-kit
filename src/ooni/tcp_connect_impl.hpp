// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_TCP_CONNECT_HPP
#define SRC_OONI_TCP_CONNECT_HPP

#include "src/ooni/errors.hpp"
#include "src/ooni/tcp_test_impl.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

class TCPConnectImpl : public TCPTestImpl {
    using TCPTestImpl::TCPTestImpl;

    TCPClient client;

    std::function<void(report::Entry)> have_entry;

  public:
    TCPConnectImpl(std::string input_filepath_, Settings options_)
        : TCPTestImpl(input_filepath_, options_) {
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
              std::function<void(report::Entry)> &&cb) {
        options["host"] = input;
        have_entry = cb;
        client = connect(options, [this]() {
            logger.debug("tcp_connect: Got response to TCP connect test");
            have_entry(entry);
        });
    }
};

} // namespace ooni
} // namespace mk
#endif
