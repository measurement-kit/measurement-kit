// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/net/transport.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"

namespace mk {
namespace ooni {

void tcp_connect(std::string input, Settings options,
                 Callback<SharedPtr<nlohmann::json>> callback, SharedPtr<Reactor> reactor,
                 SharedPtr<Logger> logger) {
    SharedPtr<nlohmann::json> entry(new nlohmann::json);
    (*entry)["connection"] = nullptr;
    // Note: unlike ooni-probe, here we also accept endpoints where the port
    // is not specified, defaulting to 80 in such case.
    ErrorOr<net::Endpoint> maybe_epnt = net::parse_endpoint(input, 80);
    if (!maybe_epnt) {
        (*entry)["connection"] = maybe_epnt.as_error().reason;
        callback(entry);
        return;
    }
    options["host"] = maybe_epnt->hostname;
    options["port"] = maybe_epnt->port;
    templates::tcp_connect(options, [=](Error err, SharedPtr<net::Transport> txp) {
        logger->debug("tcp_connect: Got response to TCP connect test");
        if (err) {
            (*entry)["connection"] = err.reason;
            callback(entry);
            return;
        }
        txp->close([=]() {
            (*entry)["connection"] = "success";
            callback(entry);
        });
    }, reactor, logger);
}

} // namespace ooni
} // namespace mk
