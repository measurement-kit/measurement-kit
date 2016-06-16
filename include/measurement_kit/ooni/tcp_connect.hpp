// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_TCP_CONNECT_HPP
#define MEASUREMENT_KIT_OONI_TCP_CONNECT_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni/ooni_test.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void tcp_connect(std::string, Settings, Callback<Var<Entry>>,
                 Var<Reactor> = Reactor::global(),
                 Var<Logger> = Logger::global());

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
        tcp_connect(input, options, [=](Var<report::Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace ooni
} // namespace mk
#endif
