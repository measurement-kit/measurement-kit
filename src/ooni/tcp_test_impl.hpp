// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_OONI_TCP_TEST_HPP
#define SRC_OONI_TCP_TEST_HPP

#include <measurement_kit/net.hpp>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>

#include "src/ext/json/src/json.hpp"                // for json

#include "src/ooni/ooni_test_impl.hpp"

using json = nlohmann::json;

namespace mk {

namespace net {
class Connection;
}

namespace ooni {

class TCPTestImpl : public ooni::OoniTestImpl {
    using ooni::OoniTestImpl::OoniTestImpl;

  public:
    TCPTestImpl(std::string input_filepath_, Settings options_)
        : ooni::OoniTestImpl(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
        entry["sent"] = json::array();
        entry["received"] = json::array();
        entry["failure"] = nullptr;
    };

    TCPTestImpl(Settings options_) : TCPTestImpl("", options_) {};

    void connect(Settings options,
            std::function<void(Var<net::Transport>)> &&cb) {
        if (options["port"] == "") {
            throw std::runtime_error("Port is required");
        }
        if (options["host"] == "") {
            options["host"] = "localhost";
        }

        net::connect(options["host"], options["port"].as<int>(),
                [this, cb](Error error, Var<net::Transport> transport) {
                    if (error) {
                        entry["failure"] = error.as_ooni_error();
                    }
                    cb(transport);
                }, {}, &logger, poller);
    }
};

} // namespace ooni
} // namespace mk
#endif
