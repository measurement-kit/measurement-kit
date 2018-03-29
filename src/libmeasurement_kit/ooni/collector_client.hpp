// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_COLLECTOR_CLIENT_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_COLLECTOR_CLIENT_HPP

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
                   SharedPtr<Reactor> = Reactor::global(),
                   SharedPtr<Logger> = Logger::global());

void submit_report(std::string filepath, std::string collector_base_url,
                   std::string collector_front_domain,
                   Callback<Error> callback, Settings conf = {},
                   SharedPtr<Reactor> = Reactor::global(),
                   SharedPtr<Logger> = Logger::global());

/*
    The following APIs are used to implement `submit_report()` and could
    also be the basic bricks to open report at the beginning, update during
    the test progress, and close when test ends:
*/

void connect(Settings, Callback<Error, SharedPtr<net::Transport>>,
             SharedPtr<Reactor> = Reactor::global(), SharedPtr<Logger> = Logger::global());

void create_report(SharedPtr<net::Transport>, report::Entry,
                   Callback<Error, std::string>, Settings = {},
                   SharedPtr<Reactor> = Reactor::global(),
                   SharedPtr<Logger> = Logger::global());

void connect_and_create_report(report::Entry, Callback<Error, std::string>,
                               Settings = {}, SharedPtr<Reactor> = Reactor::global(),
                               SharedPtr<Logger> = Logger::global());

void update_report(SharedPtr<net::Transport>, std::string report_id, report::Entry,
                   Callback<Error>, Settings = {},
                   SharedPtr<Reactor> = Reactor::global(),
                   SharedPtr<Logger> = Logger::global());

void connect_and_update_report(std::string report_id, report::Entry,
                               Callback<Error>, Settings = {},
                               SharedPtr<Reactor> = Reactor::global(),
                               SharedPtr<Logger> = Logger::global());

void close_report(SharedPtr<net::Transport>, std::string report_id, Callback<Error>,
                  Settings = {}, SharedPtr<Reactor> = Reactor::global(),
                  SharedPtr<Logger> = Logger::global());

void connect_and_close_report(std::string report_id, Callback<Error>,
                              Settings = {}, SharedPtr<Reactor> = Reactor::global(),
                              SharedPtr<Logger> = Logger::global());

} // namespace collector
} // namespace mk
} // namespace ooni
#endif
