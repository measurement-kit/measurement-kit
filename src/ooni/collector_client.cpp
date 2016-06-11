// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/collector_client_impl.hpp"
#include <regex>

namespace mk {
namespace ooni {
namespace collector {

using namespace mk::http;
using namespace mk::net;
using namespace mk::report;

static const std::regex re_name{"^[A-Za-z0-9._-]+$"};
static const std::regex re_version{
    "^[0-9]+.[0-9]+(.[0-9]+(-[A-Za-z0-9._-]+)?)?$"};

static std::map<std::string, std::regex> mandatory_re{
    {"software_name", re_name},
    {"software_version", re_version},
    {"probe_asn", std::regex{"^AS[0-9]+$"}},
    {"probe_cc", std::regex{"^[A-Z]{2}$"}},
    {"test_name", re_name},
    {"test_version", re_version},
    {"data_format_version", re_version},
    {"test_start_time",
     std::regex{"^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}$"}},
};

Error valid_entry(Entry entry) {
    // TODO: also validate the optional values
    for (auto pair : mandatory_re) {
        ErrorOr<std::string> s = entry[pair.first];
        if (!s) {
            return MissingMandatoryKeyError(s.as_error());
        }
        if (!std::regex_match(*s, pair.second)) {
            return InvalidMandatoryValueError(pair.first);
        }
    }
    return NoError();
}

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
    } catch (const std::invalid_argument &) {
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
    return MK_OONI_DEFAULT_COLLECTOR_URL;
}

} // namespace collector
} // namespace mk
} // namespace ooni
