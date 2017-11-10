// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ooni/collector_client_impl.hpp"
#include <regex>

namespace mk {
namespace ooni {
namespace collector {

using namespace mk::http;
using namespace mk::net;
using namespace mk::report;

// Regex notes
// -----------
//
// 1. [the default grammar is ECMAScript](
//      http://en.cppreference.com/w/cpp/regex/basic_regex
//    )
//
// 2. the `re_version` regexp is copied from [sindresorhus/semver-regex](
//       https://github.com/sindresorhus/semver-regex/blob/624a91ef62e593abebbf8f411449ab0d257eac4d/index.js
//    )
//
// 3. R"(...)" delimits a [raw characters sequence](
//      http://en.cppreference.com/w/cpp/language/string_literal
//    )

static const std::regex re_name{R"(^[A-Za-z0-9._-]+$)"};
static const std::regex re_version{
    R"(^v?(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)(?:-[\da-z\-]+(?:\.[\da-z\-]+)*)?(?:\+[\da-z\-]+(?:\.[\da-z\-]+)*)?$)",
    std::regex::icase};

static std::map<std::string, std::regex> mandatory_re{
    {"software_name", re_name},
    {"software_version", re_version},
    {"probe_asn", std::regex{R"(^AS[0-9]+$)"}},
    {"probe_cc", std::regex{R"(^[A-Z]{2}$)"}},
    {"test_name", re_name},
    {"test_version", re_version},
    {"data_format_version", re_version},
    {"test_start_time",
     std::regex{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}$)"}},
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

void post(SharedPtr<Transport> transport, std::string url_extra, std::string body,
          Callback<Error, Json> callback, Settings conf,
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
        return FileEofError();
    }
    if (!file->good()) {
        logger->warn("I/O error reading file");
        return FileIoError();
    }
    logger->debug("Read line from report: %s", line.c_str());
    Entry entry;
    // Works because we are using Json::json() as Entry::Entry()
    auto e = json_process(line, [&](auto j) { entry = j; });
    if (e != NoError()) {
        return e;
    }
    return entry;
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
