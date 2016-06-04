// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/collector_client_impl.hpp"

namespace mk {
namespace ooni {
namespace collector {

using namespace mk::http;
using namespace mk::net;

void post(Var<Transport> transport, std::string url_extra, std::string body,
          Callback<Error, nlohmann::json> callback, Settings conf,
          Var<Reactor> reactor, Var<Logger> logger) {
    post_impl(transport, url_extra, body, callback, conf, reactor, logger);
}

void connect(Settings settings, Callback<Error, Var<Transport>> callback,
             Var<Reactor> reactor, Var<Logger> logger) {
    connect_impl(settings, callback, reactor, logger);
}

void create_report(Var<Transport> transport, Entry entry,
                   Callback<Error, std::string> callback, Settings settings,
                   Var<Reactor> reactor, Var<Logger> logger) {
    create_report_impl(transport, entry, callback, settings, reactor, logger);
}

void update_report(Var<Transport> transport, std::string report_id, Entry entry,
                   Callback<Error> callback, Settings settings,
                   Var<Reactor> reactor, Var<Logger> logger) {
    update_report_impl(transport, report_id, entry, callback, settings, reactor,
                       logger);
}

void close_report(Var<Transport> transport, std::string report_id,
                  Callback<Error> callback, Settings settings,
                  Var<Reactor> reactor, Var<Logger> logger) {
    close_report_impl(transport, report_id, callback, settings, reactor,
                      logger);
}

ErrorOr<Entry> get_next_entry(Var<std::istream> file, Var<Logger> logger) {
    std::string line;
    std::getline(*file, line);
    if (file->eof()) {
        logger->info("End of file found");
        return FileEofError();
    }
    if (!file->good()) {
        logger->warn("I/O error reading file");
        return FileIoError();
    }
    logger->debug("Read line from report: %s", line.c_str());
    try {
        // Works because we are using nlohmann::json::json() as Entry::Entry()
        return Entry(nlohmann::json::parse(line));
    } catch (std::invalid_argument &) {
        return JsonParseError();
    }
    /* NOTREACHED */
}

void submit_report(std::string filepath, std::string collector_base_url,
                   Callback<Error> callback, Settings conf,
                   Var<Reactor> reactor, Var<Logger> logger) {
    submit_report_impl(filepath, collector_base_url, callback, conf, reactor,
                       logger);
}

std::string default_collector_url() {
    return "http://a.collector.test.ooni.io";
}

} // namespace collector
} // namespace mk
} // namespace ooni
