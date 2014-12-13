#include "ooni/tcp_test.hpp"

using namespace ight::ooni::tcp_test;

TCPClient
TCPTest::connect(ight::common::Settings options, std::function<void()>&& cb)
{
    if (options["port"] == "") {
        throw std::runtime_error("Port is required");
    }
    if (options["host"] == "") {
        options["host"] = "localhost";
    }

    auto tcp_client = TCPClient(options["host"], options["port"]);

    //
    // FIXME The lifecycle of `tcp_client` *is not* bound to
    // the lifecycle of `this` but we pass `this` as an argument
    // to a `tcp_client` callback.
    //

    tcp_client.on("connect", [this, cb]() {
        entry["connection"] = "success";
        cb();
    });

    tcp_client.on("error", [this, cb](IghtError e) {
        entry["error_code"] = e.error;
        entry["connection"] = "failed";
        cb();
    });

    return tcp_client;
};
