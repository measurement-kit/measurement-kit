// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_TCP_CONNECT_HPP
#define SRC_OONI_TCP_CONNECT_HPP

#include "src/ooni/tcp_test_impl.hpp"

namespace mk {
namespace ooni {

class TCPConnectImpl : public TCPTestImpl {
    using TCPTestImpl::TCPTestImpl;

  public:
    TCPConnectImpl(std::string input_filepath_, Settings options_)
        : TCPTestImpl(input_filepath_, options_) {
        test_name = "tcp_connect";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options,
              std::function<void(report::Entry)> cb) {
        options["host"] = input;
        connect(options, [this, cb](Error err, Var<net::Transport> txp) {
            logger->debug("tcp_connect: Got response to TCP connect test");
            if (err) {
                cb(report::Entry{{"connection", err.as_ooni_error()}});
                return;
            }
            txp->close([this, cb]() {
                cb(report::Entry{{"connection", "success"}});
            });
        });
    }
};

} // namespace ooni
} // namespace mk
#endif
