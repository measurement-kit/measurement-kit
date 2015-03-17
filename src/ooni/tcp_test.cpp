#include <ight/ooni/tcp_test.hpp>

using namespace ight::common::error;
using namespace ight::common::pointer;
using namespace ight::common::settings;
using namespace ight::ooni::tcp_test;

TCPClient
TCPTest::connect(Settings options, std::function<void()>&& cb)
{
    if (options["port"] == "") {
        throw std::runtime_error("Port is required");
    }
    if (options["host"] == "") {
        options["host"] = "localhost";
    }

    auto connection = std::make_shared<Connection>("PF_UNSPEC",
            options["host"].c_str(), options["port"].c_str());

    //
    // FIXME The connection and this are bound in the
    // callbacks below, but they have possibly different
    // life cycles, which is &disaster.
    //

    connection->on_error([cb, this](Error e) {
        entry["error_code"] = e.error;
        entry["connection"] = "failed";
        cb();
    });
    connection->on_connect([this, cb]() {
        entry["connection"] = "success";
        cb();
    });

    return connection;
}
