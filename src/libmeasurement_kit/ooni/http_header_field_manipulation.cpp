// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/constants.hpp"
#include "../ooni/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void http_header_field_manipulation(std::string input, Settings options,
                   Callback<Var<report::Entry>> callback,
                   Var<Reactor> reactor, Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    (*entry)["tampering"] = Entry::object();

    options["http/url"] = options["backend"];
    std::string body = ""; // spec says this is always a GET, so no body

    std::string random_host = random_str(15) + ".com";
    std::string random_ua = random_choice(constants::COMMON_USER_AGENTS);

    http::Headers headers = { {randomly_capitalize("host"),
                               random_host},
                              {randomly_capitalize("user-agent"),
                               random_ua},
                              {randomly_capitalize("accept"),
                               "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
                              {randomly_capitalize("accept-encoding"),
                               "gzip,deflate,sdch"},
                              {randomly_capitalize("accept-language"),
                               "en-US,en;q=0.8"},
                              {randomly_capitalize("accept-charset"),
                               "ISO-8859-1,utf-8;q=0.7,*;q=0.3"}
                            };

    templates::http_request(entry, options, headers, body,
                            [=](Error err, Var<http::Response> response) {
                                if (err) {
                                    logger->debug(
                                        "http_header_field_manipulation: http-request error: %s",
                                        err.explain().c_str());

                                    (*entry)["failure"] =
                                        err.as_ooni_error();
                                }

                                if (!response) {
                                    logger->warn("null response");
                                } else {
                                    compare_headers_response(headers, response,
                                                             entry, logger);
                                }

                                callback(entry);
                            },
                            reactor, logger);
}

} // namespace ooni
} // namespace mk
