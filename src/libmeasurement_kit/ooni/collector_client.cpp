// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/collector_client_impl.hpp"
#include "src/libmeasurement_kit/regexp/regexp.hpp"

namespace mk {
namespace ooni {
namespace collector {

using namespace mk::http;
using namespace mk::net;
using namespace mk::report;

static std::map<std::string, bool(*)(const std::string &)> mandatory_re{
    {"software_name", regexp::valid_nettest_name},
    {"software_version", regexp::valid_nettest_version},
    {"probe_asn", regexp::valid_probe_asn},
    {"probe_cc", regexp::valid_country_code},
    {"test_name", regexp::valid_nettest_name},
    {"test_version", regexp::valid_nettest_version},
    {"data_format_version", regexp::valid_nettest_version},
    {"test_start_time", regexp::valid_test_start_time},
};

Error valid_entry(Entry entry) {
    // TODO: also validate the optional values
    for (auto pair : mandatory_re) {
        ErrorOr<std::string> s = entry[pair.first];
        if (!s) {
            return MissingMandatoryKeyError(s.as_error());
        }
        if (!(*pair.second)(*s)) {
            return InvalidMandatoryValueError(pair.first);
        }
    }
    return NoError();
}

void post(SharedPtr<Transport> transport, std::string url_extra, std::string body,
          Callback<Error, nlohmann::json> callback, Settings conf,
          SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    post_impl(transport, url_extra, body, callback, conf, reactor, logger);
}

void connect(Settings settings, Callback<Error, SharedPtr<Transport>> callback,
             SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    connect_impl(settings, callback, reactor, logger);
}

void create_report(SharedPtr<Transport> transport, Entry entry,
                   Callback<Error, std::string> callback, Settings settings,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    create_report_impl(transport, entry, callback, settings, reactor, logger);
}

void connect_and_create_report(report::Entry entry,
                               Callback<Error, std::string> callback,
                               Settings settings, SharedPtr<Reactor> reactor,
                               SharedPtr<Logger> logger) {
    connect_and_create_report_impl(entry, callback, settings, reactor, logger);
}

void update_report(SharedPtr<Transport> transport, std::string report_id, Entry entry,
                   Callback<Error> callback, Settings settings,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    update_report_impl(transport, report_id, entry, callback, settings, reactor,
                       logger);
}

void connect_and_update_report(std::string report_id, report::Entry entry,
                               Callback<Error> callback, Settings settings,
                               SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    connect_and_update_report_impl(report_id, entry, callback, settings,
                                   reactor, logger);
}

void close_report(SharedPtr<Transport> transport, std::string report_id,
                  Callback<Error> callback, Settings settings,
                  SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    close_report_impl(transport, report_id, callback, settings, reactor,
                      logger);
}

void connect_and_close_report(std::string report_id, Callback<Error> callback,
                              Settings settings, SharedPtr<Reactor> reactor,
                              SharedPtr<Logger> logger) {
    connect_and_close_report_impl(report_id, callback, settings, reactor,
                                  logger);
}

ErrorOr<Entry> get_next_entry(SharedPtr<std::istream> file, SharedPtr<Logger> logger) {
    std::string line;
    std::getline(*file, line);
    if (file->eof()) {
        logger->info("End of file found");
        return {FileEofError(), {}};
    }
    if (!file->good()) {
        logger->warn("I/O error reading file");
        return {FileIoError(), {}};
    }
    logger->debug("Read line from report: %s", line.c_str());
    Entry entry;
    Error e = NoError();
    try {
        // Works because we are using nlohmann::json::json() as Entry::Entry()
        entry = nlohmann::json::parse(line);
    } catch (const std::exception &) {
        e = JsonParseError();
    }
    if (e != NoError()) {
        return {e, {}};
    }
    return {NoError(), entry};
}

void submit_report(std::string filepath, std::string collector_base_url,
                   Callback<Error> callback, Settings conf,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    submit_report_impl(filepath, collector_base_url, "",
                       callback, conf, reactor,
                       logger);
}

void submit_report(std::string filepath, std::string collector_base_url,
                   std::string collector_front_domain,
                   Callback<Error> callback, Settings conf,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    submit_report_impl(filepath, collector_base_url,
                       collector_front_domain,
                       callback, conf, reactor,
                       logger);
}

std::string production_collector_url() {
    return MK_OONI_PRODUCTION_COLLECTOR_URL;
}

std::string testing_collector_url() {
    return MK_OONI_TESTING_COLLECTOR_URL;
}

} // namespace collector
} // namespace mk
} // namespace ooni
