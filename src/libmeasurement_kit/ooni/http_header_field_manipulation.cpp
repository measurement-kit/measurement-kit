// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/http_header_field_manipulation.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void compare_headers_response(http::Headers headers,
                             SharedPtr<http::Response> response, SharedPtr<report::Entry> entry,
                             SharedPtr<Logger> logger) {
    if (response->body.empty()) {
        logger->warn("empty response body");
        (*entry)["tampering"]["total"] = true;
        (*entry)["tampering"]["request_line_capitalization"] = true;
        return;
    }

    Json resp;
    try {
        resp = Json::parse(response->body);
    } catch (const std::invalid_argument &) {
        logger->warn("response body not valid JSON");
        (*entry)["tampering"]["total"] = true;
        (*entry)["tampering"]["request_line_capitalization"] = true;
        return;
    }

    (*entry)["tampering"]["total"] = false;

    if (resp.find("request_line") == resp.end()) {
        (*entry)["tampering"]["request_line_capitalization"] = true;
    } else if (resp["request_line"] != "GET / HTTP/1.1") {
        (*entry)["tampering"]["request_line_capitalization"] = true;
    } else {
        (*entry)["tampering"]["request_line_capitalization"] = false;
    }

    // ooni-probe behavior to report header keys in the request or response
    // but not both. (case-sensitive, and ignoring values)
    Json resp_headers = resp["headers_dict"];
    std::set<std::string> req_keys, resp_keys, diff;
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        req_keys.insert(it->first);
        logger->debug("ins %s in req_keys", it->first.c_str());
    }
    for (auto it = resp_headers.begin(); it != resp_headers.end(); ++it) {
        resp_keys.insert(it.key());
        logger->debug("ins %s in resp_keys", it.key().c_str());
    }

    std::set_difference(req_keys.begin(), req_keys.end(),
                        resp_keys.begin(), resp_keys.end(),
                        std::inserter(diff, diff.begin()));
    std::set_difference(resp_keys.begin(), resp_keys.end(),
                        req_keys.begin(), req_keys.end(),
                        std::inserter(diff, diff.begin()));
    (*entry)["tampering"]["header_name_diff"] = diff;
    (*entry)["tampering"]["header_field_name"] = !diff.empty();
}

void http_header_field_manipulation(std::string /*input*/, Settings options,
                                    Callback<SharedPtr<report::Entry>> callback,
                                    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<Entry> entry(new Entry);
    (*entry)["tampering"] = Entry::object();
    (*entry)["tampering"]["total"] = nullptr;
    (*entry)["tampering"]["request_line_capitalization"] = nullptr;
    (*entry)["tampering"]["header_name_diff"] = nullptr;
    (*entry)["tampering"]["header_field_name"] = nullptr;
    (*entry)["failure"] = nullptr;

    options["http/url"] = options["backend"];
    std::string body = ""; // spec says this is always a GET, so no body

    std::string random_host = random_str(15) + ".com";
    std::string random_ua = random_choice(constants::COMMON_USER_AGENTS);

    http::Headers headers = {
        {randomly_capitalize("host"), random_host},
        {randomly_capitalize("user-agent"), random_ua},
        {randomly_capitalize("accept"),
         "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
        {randomly_capitalize("accept-encoding"), "gzip,deflate,sdch"},
        {randomly_capitalize("accept-language"), "en-US,en;q=0.8"},
        {randomly_capitalize("accept-charset"),
         "ISO-8859-1,utf-8;q=0.7,*;q=0.3"}};

    templates::http_request(
        entry, options, headers, body,
        [=](Error err, SharedPtr<http::Response> response) {
            if (err) {
                logger->debug(
                    "http_header_field_manipulation: http-request error: %s",
                    err.what());
                (*entry)["failure"] = err.reason;
            }

            if (!response) {
                logger->warn("null response");
            } else {
                try {
                    compare_headers_response(headers, response, entry, logger);
                } catch (const std::exception &exc) {
                    (*entry)["failure"] = exc.what(); // XXX
                    logger->warn("exception in compare_headers_response(): %s",
                                 exc.what());
                }
            }

            callback(entry);
        },
        reactor, logger);
}

} // namespace ooni
} // namespace mk
