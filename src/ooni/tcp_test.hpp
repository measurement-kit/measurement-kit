// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_TEST_HPP

#include <measurement_kit/net/buffer.hpp>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>

#include "src/ooni/net_test.hpp"

namespace measurement_kit {
namespace net {
class Connection;
}
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

typedef Var<Connection> TCPClient; /* XXX */

class TCPTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

  public:
    TCPTest(std::string input_filepath_, Settings options_)
        : ooni::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };

    TCPClient connect(Settings options, std::function<void()> &&cb);
};
}
}
#endif
