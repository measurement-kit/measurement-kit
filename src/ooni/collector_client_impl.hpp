// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_OONI_COLLECTOR_CLIENT_IMPL_HPP
#define SRC_OONI_COLLECTOR_CLIENT_IMPL_HPP

// This file implements the OONI collector client protocol
// See <https://github.com/TheTorProject/ooni-spec/blob/master/oonib.md>

#include <fstream>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace collector {

using namespace mk::http;
using namespace mk::net;
using namespace mk::report;

/*
 _          _
| |__   ___| |_ __   ___ _ __ ___
| '_ \ / _ \ | '_ \ / _ \ '__/ __|
| | | |  __/ | |_) |  __/ |  \__ \
|_| |_|\___|_| .__/ \___|_|  |___/
             |_|
*/

void post(Var<Transport> transport, std::string url_extra, std::string body,
          Callback<Error, nlohmann::json> callback, Settings conf = {},
          Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

template <MK_MOCK_NAMESPACE(http, request_sendrecv)>
void post_impl(Var<Transport> transport, std::string append_to_url,
               std::string body, Callback<Error, nlohmann::json> callback,
               Settings settings, Var<Reactor> reactor, Var<Logger> logger) {
    if (settings.find("collector_base_url") == settings.end()) {
        callback(MissingCollectorBaseUrlError(), nullptr);
        return;
    }
    std::string url = settings["collector_base_url"];
    url += append_to_url;
    settings["http/url"] = url;
    settings["http/method"] = "POST";
    Headers headers;
    if (body != "") {
        headers["Content-Type"] = "application/json";
    }
    logger->debug("POST %s '%s'...", url.c_str(), body.c_str());
    http_request_sendrecv(transport, settings, headers, body,
                          [=](Error err, Var<Response> response) {
                              logger->debug("POST %s '%s'... %d", url.c_str(),
                                            body.c_str(), err.code);
                              if (err) {
                                  callback(err, nullptr);
                                  return;
                              }
                              if (response->status_code / 100 != 2) {
                                  callback(HttpRequestFailedError(), nullptr);
                                  return;
                              }
                              // If response is empty, don't parse it
                              if (response->body == "") {
                                  callback(NoError(), nullptr);
                                  return;
                              }
                              nlohmann::json reply;
                              try {
                                  reply = nlohmann::json::parse(response->body);
                              } catch (const std::invalid_argument &) {
                                  callback(JsonParseError(), nullptr);
                                  return;
                              }
                              callback(NoError(), reply);
                          },
                          reactor, logger);
}

Error valid_entry(Entry entry);

/*
             _
  __ _ _ __ (_)
 / _` | '_ \| |
| (_| | |_) | |
 \__,_| .__/|_|
      |_|
*/

template <MK_MOCK_NAMESPACE(http, request_connect)>
void connect_impl(Settings settings, Callback<Error, Var<Transport>> callback,
                  Var<Reactor> reactor, Var<Logger> logger) {
    if (settings.find("collector_base_url") == settings.end()) {
        callback(MissingCollectorBaseUrlError(), nullptr);
        return;
    }
    settings["http/url"] = settings.at("collector_base_url");
    http_request_connect(settings, callback, reactor, logger);
}

template <MK_MOCK_NAMESPACE(collector, post)>
void create_report_impl(Var<Transport> transport, Entry entry,
                        Callback<Error, std::string> callback,
                        Settings settings, Var<Reactor> reactor,
                        Var<Logger> logger) {

    Entry request;
    Error err = valid_entry(entry);
    if (err != NoError()) {
        callback(err, "");
        return;
    }
    request["software_name"] = entry["software_name"];
    request["software_version"] = entry["software_version"];
    request["probe_asn"] = entry["probe_asn"];
    request["probe_cc"] = entry["probe_cc"];
    request["test_name"] = entry["test_name"];
    request["test_version"] = entry["test_version"];
    request["test_start_time"] = entry["test_start_time"];
    if (entry["input_hashes"] != nullptr) {
        request["input_hashes"] = entry["input_hashes"];
    } else {
        request["input_hashes"] = Entry::array();
    }
    request["data_format_version"] = entry["data_format_version"];
    request["format"] = "json";
    std::string body = request.dump();

    collector_post(transport, "/report", body,
                   [=](Error err, nlohmann::json reply) {
                       if (err) {
                           callback(err, "");
                           return;
                       }
                       std::string report_id;
                       try {
                           report_id = reply.at("report_id");
                       } catch (const std::domain_error &) {
                           callback(JsonDomainError(), "");
                           return;
                       } catch (const std::out_of_range &) {
                           callback(JsonKeyError(), "");
                           return;
                       }
                       callback(NoError(), report_id);
                   },
                   settings, reactor, logger);
}

template <MK_MOCK_NAMESPACE(collector, post)>
void update_report_impl(Var<Transport> transport, std::string report_id,
                        Entry entry, Callback<Error> callback,
                        Settings settings, Var<Reactor> reactor,
                        Var<Logger> logger) {
    Error err = valid_entry(entry);
    if (err != NoError()) {
        callback(err);
        return;
    }
    Entry request{{"format", "json"}};
    request["content"] = entry;
    std::string body = request.dump();
    collector_post(transport, "/report/" + report_id, body,
                   [=](Error err, nlohmann::json) {
                       callback(err);
                   },
                   settings, reactor, logger);
}

template <MK_MOCK_NAMESPACE(collector, post)>
void close_report_impl(Var<Transport> transport, std::string report_id,
                       Callback<Error> callback, Settings settings,
                       Var<Reactor> reactor, Var<Logger> logger) {
    // Here we log at level INFO so we can save one extra lambda
    // below to just tell the user how closing report went
    logger->info("closing report...");
    collector_post(transport, "/report/" + report_id + "/close", "",
                   [=](Error err, nlohmann::json) {
                       logger->info("closing report... %d", err.code);
                       callback(err);
                   },
                   settings, reactor, logger);
}

ErrorOr<Entry> get_next_entry(Var<std::istream> file, Var<Logger> logger);

template <MK_MOCK_NAMESPACE(collector, update_report),
          MK_MOCK_NAMESPACE(collector, get_next_entry)>
void update_and_fetch_next_impl(Var<std::istream> file, Var<Transport> txp,
                                std::string report_id, int line, Entry entry,
                                Callback<Error> callback, Settings settings,
                                Var<Reactor> reactor, Var<Logger> logger) {
    logger->info("adding entry report #%d...", line);
    collector_update_report(
        txp, report_id, entry,
        [=](Error err) {
            logger->info("adding entry report #%d... %d", line, err.code);
            if (err) {
                txp->close([=]() { callback(err); });
                return;
            }
            // After #644 bug and fix, I prefer to always break explicit
            // recursion by using the call_soon() pattern
            logger->debug("scheduling read of next entry...");
            reactor->call_soon([=]() {
                logger->debug("reading next entry");
                ErrorOr<Entry> entry = collector_get_next_entry(file, logger);
                if (!entry) {
                    if (entry.as_error() != FileEofError()) {
                        txp->close([=]() { callback(entry.as_error()); });
                        return;
                    }
                    close_report(
                        txp, report_id,
                        [=](Error err) {
                            txp->close([=]() { callback(err); });
                        },
                        settings, reactor, logger);
                    return;
                }
                update_and_fetch_next_impl<collector_update_report,
                                           collector_get_next_entry>(
                    file, txp, report_id, line + 1, *entry, callback, settings,
                    reactor, logger);
            });
        },
        settings, reactor, logger);
}

template <MK_MOCK_NAMESPACE(collector, get_next_entry),
          MK_MOCK_NAMESPACE(collector, connect),
          MK_MOCK_NAMESPACE(collector, create_report)>
void submit_report_impl(std::string filepath, std::string collector_base_url,
                        Callback<Error> callback, Settings settings,
                        Var<Reactor> reactor, Var<Logger> logger) {

    Var<std::istream> file(new std::ifstream(filepath));
    if (!file->good()) {
        callback(CannotOpenReportError());
        return;
    }
    ErrorOr<Entry> entry = collector_get_next_entry(file, logger);
    if (!entry) {
        callback(entry.as_error());
        return;
    }

    settings["collector_base_url"] = collector_base_url;
    logger->info("connecting to collector %s...", collector_base_url.c_str());
    collector_connect(
        settings,
        [=](Error err, Var<Transport> txp) {
            logger->info("connecting to collector %s... %d",
                         collector_base_url.c_str(), err.code);
            if (err) {
                callback(err);
                return;
            }
            logger->info("creating report...");
            collector_create_report(
                txp, *entry,
                [=](Error err, std::string report_id) {
                    logger->info("creating report... %d", err.code);
                    if (err) {
                        txp->close([=]() { callback(err); });
                        return;
                    }
                    update_and_fetch_next_impl(file, txp, report_id, 1, *entry,
                                               callback, settings, reactor,
                                               logger);
                },
                settings, reactor, logger);
        },
        reactor, logger);
}

} // namespace collector
} // namespace mk
} // namespace ooni
#endif
