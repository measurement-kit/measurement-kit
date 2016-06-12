// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_TCP_CONNECT_HPP
#define SRC_OONI_TCP_CONNECT_HPP

#include "src/ooni/ooni_test_impl.hpp"

namespace mk {
namespace ooni {

class TCPConnectImpl : public OoniTestImpl {
  public:
    TCPConnectImpl(std::string input_filepath_, Settings options_)
        : OoniTestImpl(input_filepath_, options_) {
        test_name = "tcp_connect";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options, Callback<report::Entry> cb) {
        tcp_connect(input, options, [=](Var<report::Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }
};

} // namespace ooni
} // namespace mk
#endif
