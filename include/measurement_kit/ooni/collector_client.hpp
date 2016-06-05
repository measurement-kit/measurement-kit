// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_BACKEND_CLIENT_HPP
#define MEASUREMENT_KIT_OONI_BACKEND_CLIENT_HPP

#include <measurement_kit/http.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace collector {

/*
    To submit a file report, pass it to `submit_report()`:
*/

#define MK_OONI_DEFAULT_COLLECTOR_URL "http://a.collector.test.ooni.io"

std::string default_collector_url();

void submit_report(std::string filepath, std::string collector_base_url,
                   Callback<Error> callback, Settings conf = {},
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

/*
    The following APIs are used to implement `submit_report()` and could
    also be the basic bricks to open report at the beginning, update during
    the test progress, and close when test ends:
*/

void connect(Settings, Callback<Error, Var<net::Transport>>,
             Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

void create_report(Var<net::Transport>, report::Entry,
                   Callback<Error, std::string>, Settings = {},
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void update_report(Var<net::Transport>, std::string report_id, report::Entry,
                   Callback<Error>, Settings = {},
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void close_report(Var<net::Transport>, std::string report_id, Callback<Error>,
                  Settings = {}, Var<Reactor> = Reactor::global(),
                  Var<Logger> = Logger::global());

} // namespace collector
} // namespace mk
} // namespace ooni
#endif
