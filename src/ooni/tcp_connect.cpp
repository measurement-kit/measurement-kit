#include <ight/ooni/tcp_connect.hpp>

using namespace ight::common::settings;
using namespace ight::ooni::tcp_connect;

void
TCPConnect::main(std::string input, Settings options,
                 std::function<void(ReportEntry)>&& cb)
{
    options["host"] = input;
    have_entry = cb;
    client = connect(options, [this]() {
        ight_debug("Got response to TCP connect test");
        have_entry(entry);
    });
}
