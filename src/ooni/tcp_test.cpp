// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/tcp_test.hpp"
#include "src/net/connection.hpp"

namespace mk {
namespace ooni {

TCPClient TCPTest::connect(Settings options, std::function<void()> &&cb) {
    if (options["port"] == "") {
        throw std::runtime_error("Port is required");
    }
    if (options["host"] == "") {
        options["host"] = "localhost";
    }

    auto connection = std::make_shared<net::Connection>(
        "PF_UNSPEC", options["host"].c_str(), options["port"].c_str());

    //
    // FIXME The connection and this are bound in the
    // callbacks below, but they have possibly different
    // life cycles, which is &disaster.
    //

    connection->on_error([cb, this](Error e) {
        entry["error_code"] = (int)e;
        entry["connection"] = "failed";
        cb();
    });
    connection->on_connect([this, cb]() {
        entry["connection"] = "success";
        cb();
    });

    return connection;
}

} // namespace ooni
} // namespace mk
