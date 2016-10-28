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

using namespace mk::report;

/*
    To submit a file report, use one of the following collectors. By default
    the library uses the `testing` collector which is a HTTPS service running
    on Heroku that discards all the input it receives.
*/

#define MK_OONI_PRODUCTION_COLLECTOR_URL "https://a.collector.ooni.io:4441"
#define MK_OONI_TESTING_COLLECTOR_URL "https://b.collector.test.ooni.io:4441"

std::string production_collector_url();
std::string testing_collector_url();

// Backward compatibility for the v0.3.x series
#define MK_OONI_DEFAULT_COLLECTOR_URL MK_OONI_PRODUCTION_COLLECTOR_URL
inline std::string default_collector_url() {
    return production_collector_url();
}

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

void create_report(Var<net::Transport>, Entry,
                   Callback<Error, std::string>, Settings = {},
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void connect_and_create_report(Entry, Callback<Error, std::string>,
                               Settings = {}, Var<Reactor> = Reactor::global(),
                               Var<Logger> = Logger::global());

void update_report(Var<net::Transport>, std::string report_id, Entry,
                   Callback<Error>, Settings = {},
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void connect_and_update_report(std::string report_id, Entry,
                               Callback<Error>, Settings = {},
                               Var<Reactor> = Reactor::global(),
                               Var<Logger> = Logger::global());

void close_report(Var<net::Transport>, std::string report_id, Callback<Error>,
                  Settings = {}, Var<Reactor> = Reactor::global(),
                  Var<Logger> = Logger::global());

void connect_and_close_report(std::string report_id, Callback<Error>,
                              Settings = {}, Var<Reactor> = Reactor::global(),
                              Var<Logger> = Logger::global());

} // namespace collector
} // namespace mk
} // namespace ooni
#endif
