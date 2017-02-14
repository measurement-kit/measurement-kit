// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void meek_fronting(std::string input, Settings options,
                   Callback<Var<report::Entry>> callback,
                   Var<Reactor> reactor, Var<Logger> logger) {
    Var<Entry> entry(new Entry);

    std::list<std::string> outer_inner = split(input, ":");
    if (outer_inner.size() != 2) {
        return; /* XXX use more specific error */
    }
    // XXX: We should make sure that we remove leading and trailing whitespaces
    // url parsing methods require a schema
    std::string outer_host = "https://" + outer_inner.front();
    std::string inner_host = "https://" + outer_inner.back();

    ErrorOr<http::Url> outer_url = mk::http::parse_url_noexcept(outer_host);
    ErrorOr<http::Url> inner_url = mk::http::parse_url_noexcept(inner_host);

    if (!outer_url || !inner_url) {
        logger->debug("Invalid url: '%s' or '%s'", outer_host.c_str(),
                      inner_host.c_str());
        callback(entry);
        return;
    }

    options["http/url"] = "https://" + outer_url->address;
    options["net/ssl"] = true;
    http::Headers headers = { {"Host", inner_url->address} };
    std::string body = ""; // spec says this is always a GET, so no body

    logger->debug("Connecting to outer host %s and requesting inner url %s.",
                  outer_url->address.c_str(),
                  inner_url->address.c_str());

    templates::http_request(entry, options, headers, body,
                            [=](Error err, Var<http::Response> response) {
                                if (err) {
                                    logger->debug(
                                        "meek_fronting: http-request error: %s",
                                        err.explain().c_str());

                                    (*entry)["meek_fronting_failure"] =
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
