// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_TEST_HPP

#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/connection.hpp>

#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/log.hpp>

#include <measurement_kit/ooni/net_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

typedef SharedPointer<Connection> TCPClient;           /* XXX */

class TCPTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

public:
    TCPTest(std::string input_filepath_, Settings options_) :
      ooni::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };

    TCPClient connect(Settings options,
            std::function<void()>&& cb);
};

}}
#endif
