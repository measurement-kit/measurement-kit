// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_TCP_CONNECT_HPP
#define MEASUREMENT_KIT_NETTESTS_TCP_CONNECT_HPP

#include <measurement_kit/nettests/ooni_test.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

class TcpConnect : public OoniTest {
  public:
    TcpConnect() : TcpConnect("", {}) {}
    TcpConnect(std::string f, Settings o) : OoniTest(f, o) {
        test_name = "tcp_connect";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options,
              Callback<report::Entry> cb) override {
        ooni::tcp_connect(input, options, [=](Var<report::Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace nettests
} // namespace mk
#endif
