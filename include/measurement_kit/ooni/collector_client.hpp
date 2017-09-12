// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_BACKEND_CLIENT_HPP
#define MEASUREMENT_KIT_OONI_BACKEND_CLIENT_HPP

#include <measurement_kit/http.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace collector {

/*
    To submit a file report, use one of the following collectors. By default
    the library uses the `production` collector.
*/

#define MK_OONI_PRODUCTION_COLLECTOR_URL "https://b.collector.ooni.io"
#define MK_OONI_TESTING_COLLECTOR_URL "https://b.collector.test.ooni.io:4441"

std::string production_collector_url();
std::string testing_collector_url();

void submit_report(std::string filepath, std::string collector_base_url,
                   Callback<Error> callback, Settings conf = {},
                   Reactor = Reactor::global(),
                   Var<Logger> = Logger::global());

void submit_report(std::string filepath, std::string collector_base_url,
                   std::string collector_front_domain,
                   Callback<Error> callback, Settings conf = {},
                   Reactor = Reactor::global(),
                   Var<Logger> = Logger::global());

/*
    The following APIs are used to implement `submit_report()` and could
    also be the basic bricks to open report at the beginning, update during
    the test progress, and close when test ends:
*/

void connect(Settings, Callback<Error, Var<net::Transport>>,
             Reactor = Reactor::global(), Var<Logger> = Logger::global());

void create_report(Var<net::Transport>, report::Entry,
                   Callback<Error, std::string>, Settings = {},
                   Reactor = Reactor::global(),
                   Var<Logger> = Logger::global());

void connect_and_create_report(report::Entry, Callback<Error, std::string>,
                               Settings = {}, Reactor = Reactor::global(),
                               Var<Logger> = Logger::global());

void update_report(Var<net::Transport>, std::string report_id, report::Entry,
                   Callback<Error>, Settings = {},
                   Reactor = Reactor::global(),
                   Var<Logger> = Logger::global());

void connect_and_update_report(std::string report_id, report::Entry,
                               Callback<Error>, Settings = {},
                               Reactor = Reactor::global(),
                               Var<Logger> = Logger::global());

void close_report(Var<net::Transport>, std::string report_id, Callback<Error>,
                  Settings = {}, Reactor = Reactor::global(),
                  Var<Logger> = Logger::global());

void connect_and_close_report(std::string report_id, Callback<Error>,
                              Settings = {}, Reactor = Reactor::global(),
                              Var<Logger> = Logger::global());

} // namespace collector
} // namespace mk
} // namespace ooni
#endif
