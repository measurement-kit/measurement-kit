// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static std::vector<std::string> possible_uas = {
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7",
    "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3 1 2 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Mobile/7D11",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7 (.NET CLR 3.5.30729))",
};

static std::string random_choice(std::vector<std::string> inputs) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(inputs.begin(), inputs.end(), g);
    return inputs[0];
}

static std::string randomly_capitalize(std::string input) {
    std::random_device rd;
    std::mt19937 g(rd());
    for(auto &c : input) {
        if(g()%2 == 0) {
            c = toupper(c);
        }
    }
    return input;
}

void http_header_field_manipulation(std::string input, Settings options,
                   Callback<Var<report::Entry>> callback,
                   Var<Reactor> reactor, Var<Logger> logger) {
    Var<Entry> entry(new Entry);

    options["http/url"] = "http://127.0.0.1:2000";
    std::string body = ""; // spec says this is always a GET, so no body

    std::string random_host = random_str(15) + ".com";
    std::string random_ua = random_choice(possible_uas);

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

                                    (*entry)["http_header_field_manipulation_error"] =
                                        err.as_ooni_error();
                                }

                                if (!response) {
                                    logger->warn("null response");
                                }

                                callback(entry);
                            },
                            reactor, logger);
}

} // namespace ooni
} // namespace mk
