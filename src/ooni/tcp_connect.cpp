// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni/tcp_connect.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;

void
TCPConnect::main(std::string input, Settings options,
                 std::function<void(ReportEntry)>&& cb)
{
    options["host"] = input;
    have_entry = cb;
    client = connect(options, [this]() {
        logger->debug("tcp_connect: Got response to TCP connect test");
        have_entry(entry);
    });
}

}}
