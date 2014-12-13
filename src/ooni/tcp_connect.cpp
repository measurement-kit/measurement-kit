#include "ooni/tcp_connect.hpp"

using namespace ight::ooni::tcp_connect;

void
TCPConnect::main(std::string input, ight::common::Settings options,
                 std::function<void(ReportEntry)>&& cb)
{
    options["host"] = input;
    client = connect(options, [=]() {
        ight_debug("Got response to TCP connect test");
        cb(entry);
    });
}
