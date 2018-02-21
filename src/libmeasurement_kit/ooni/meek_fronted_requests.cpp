// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include "src/libmeasurement_kit/report/entry.hpp"

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void meek_fronted_requests(std::string input, Settings options,
                   Callback<SharedPtr<report::Entry>> callback,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<Entry> entry(new Entry);

    std::string expected_body, outer_host, inner_host;

    if (options["expected_body"].empty()) {
        expected_body = constants::MEEK_SERVER_RESPONSE;
    } else {
        expected_body = options["expected_body"];
    }

    std::list<std::string> outer_inner = split(input, ":");
    if (outer_inner.size() != 2) {
        logger->warn("Couldn't split input: %s", input.c_str());
        (*entry)["failure"] = ValueError().reason;
        callback(entry);
        return;
    }
    // XXX: We should make sure that we remove leading and trailing whitespaces
    outer_host = outer_inner.front();
    inner_host = outer_inner.back();

    // url parsing methods require a schema
    outer_host.insert(0, "https://");
    inner_host.insert(0, "https://");

    ErrorOr<http::Url> outer_url = mk::http::parse_url_noexcept(outer_host);
    ErrorOr<http::Url> inner_url = mk::http::parse_url_noexcept(inner_host);

    if (!outer_url || !inner_url) {
        logger->warn("Invalid url: %s or %s", outer_host.c_str(),
                     inner_host.c_str());
        (*entry)["failure"] = ValueError().reason;
        callback(entry);
        return;
    }

    options["http/url"] = "https://" + outer_url->address;
    http::Headers headers = { {"Host", inner_url->address} };
    std::string body = ""; // spec says this is always a GET, so no body

    logger->debug("Connecting to outer host %s and requesting inner url %s.",
                  outer_url->address.c_str(),
                  inner_url->address.c_str());

    templates::http_request(entry, options, headers, body,
                            [=](Error err, SharedPtr<http::Response> response) {
                                if (err) {
                                    logger->debug(
                                        "meek_fronted_requests: http-request error: %s",
                                        err.what());

                                    (*entry)["failure"] =
                                        err.reason;
                                }

                                if (!!response) {
                                    (*entry)["success"] =
                                        (response->body == expected_body);
                                }


                                callback(entry);
                            },
                            reactor, logger);
}

} // namespace ooni
} // namespace mk
