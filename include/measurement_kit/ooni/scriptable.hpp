// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_SCRIPTABLE_HPP
#define MEASUREMENT_KIT_OONI_SCRIPTABLE_HPP

// This module wraps OONI's tests offering several convenience
// functionality, e.g., running them in a background thread.

#include <measurement_kit/dns.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace scriptable {

/*
    Async functions. The following functions run the requested operation
    in the background thread managed by the Runner instance. The Entry is
    returned serialized as a string, again to help scriptability.
*/

void dns_injection(std::string input, Settings settings,
                   Callback<std::string> callback,
                   Var<Runner> runner = Runner::global(),
                   Var<Logger> = Logger::global());

void http_invalid_request_line(Settings settings, Callback<std::string> cb,
                               Var<Runner> runner = Runner::global(),
                               Var<Logger> logger = Logger::global());

void tcp_connect(std::string input, Settings settings,
                 Callback<std::string> callback,
                 Var<Runner> runner = Runner::global(),
                 Var<Logger> logger = Logger::global());

void web_connectivity(std::string input, Settings settings,
                      Callback<std::string> callback,
                      Var<Runner> runner = Runner::global(),
                      Var<Logger> logger = Logger::global());

} // namespace scriptable
} // namespace mk
} // namespace ooni
#endif
