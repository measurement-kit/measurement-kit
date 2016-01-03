// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_TEST_HPP

#include <measurement_kit/net/buffer.hpp>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>

#include "src/ooni/ooni_test.hpp"

namespace mk {

namespace net { class Connection; }

namespace ooni {

typedef Var<net::Connection> TCPClient; /* XXX */

class TCPTest : public ooni::OoniTest {
    using ooni::OoniTest::OoniTest;

  public:
    TCPTest(std::string input_filepath_, Settings options_)
        : ooni::OoniTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };

    TCPClient connect(Settings options, std::function<void()> &&cb);
};

} // namespace ooni
} // namespace mk
#endif
