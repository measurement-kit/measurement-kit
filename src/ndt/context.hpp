// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_CONTEXT_HPP
#define SRC_NDT_CONTEXT_HPP

#include "src/ndt/definitions.hpp"
#include <list>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ndt.hpp>
#include <measurement_kit/net.hpp>
#include <string>

namespace mk {
namespace ndt {

struct Context {
    std::string address;
    Var<net::Buffer> buff = net::Buffer::make();
    Callback<Error> callback;
    std::list<std::string> granted_suite;
    Var<Logger> logger = Logger::global();
    Var<Reactor> reactor = Reactor::global();
    int port = 0;
    Settings settings;
    int test_suite = TEST_STATUS | TEST_META | TEST_C2S | TEST_S2C;
    double timeout = 10.0;
    Var<net::Transport> txp;
};

} // namespace mk
} // namespace ndt
#endif
