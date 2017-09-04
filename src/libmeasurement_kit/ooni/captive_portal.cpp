// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/common/parallel.hpp"
#include "private/common/utils.hpp"
#include "private/ooni/constants.hpp"
#include "private/ooni/utils.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

//
// Implementation note:
// all but one of the current vendor tests are HTTP tests.
// these check either a status code or the body content.
//

typedef std::map<std::string, std::string> input_t; // Syntactic sugar

static const std::vector<input_t> &gen_http_inputs() {
    return locked_global([]() {
        static std::vector<input_t> is;
        if (is.size() <= 0) {
            input_t i;
            i["name"] = "MS HTTP Captive Portal";
            i["url"] = "http://www.msftncsi.com/ncsi.txt";
            i["body"] = "Microsoft NCSI";
            is.push_back(i);
            i.clear();
            i["name"] = "Apple HTTP Captive Portal";
            i["url"] = "http://captive.apple.com";
            i["body"] =
                  "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</"
                  "BODY></HTML>\n";
            i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
            is.push_back(i);
            i.clear();
            i["name"] = "Apple HTTP Captive Portal 2";
            i["url"] = "http://captive.apple.com/hotspot_detect.html";
            i["body"] =
                  "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</"
                  "BODY></HTML>\n";
            i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
            is.push_back(i);
            i.clear();
            i["name"] = "Android KitKat HTTP Captive Portal";
            i["url"] = "http://clients3.google.com/generate_204";
            i["status"] = "204"; // XXX: easier to convert to int later...
            is.push_back(i);
            i.clear();
            i["name"] = "Android Lollipop HTTP Captive Portal";
            i["url"] = "http://connectivitycheck.android.com/generate_204";
            i["status"] = "204";
            is.push_back(i);
            i.clear();
            i["name"] = "Android Marshmallow HTTP Captive Portal";
            i["url"] = "http://connectivitycheck.gstatic.com/generate_204";
            i["status"] = "204";
            is.push_back(i);
            i.clear();
            i["name"] = "Android Nougat HTTP Captive Portal 1";
            i["url"] = "https://www.google.com/generate_204";
            i["status"] = "204";
            i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                      "(KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
            is.push_back(i);
            i.clear();
            i["name"] = "Android Nougat HTTP Captive Portal 2";
            i["url"] = "http://www.google.com/gen_204";
            i["status"] = "204";
            i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                      "(KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
            is.push_back(i);
            i.clear();
            i["name"] = "Android Nougat HTTP Captive Portal 3";
            i["url"] = "http://play.googleapis.com/generate_204";
            i["status"] = "204";
            i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                      "(KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
            is.push_back(i);
            i.clear();
        }
        return is;
    });
}

void http_many(Var<Entry> entry, Callback<Error> all_done_cb, Settings options,
               Var<Reactor> reactor, Var<Logger> logger) {

    auto http_cb = [=](const input_t &input, Callback<Error> done_cb) {
        return [=](Error err, Var<http::Response> response) {
            // result: true means unfiltered
            Entry result = {
                  {"URL", input.at("url")},
                  {"http_status_number",
                   nullptr}, // expected status code, if there is one
                  {"http_status_summary",
                   nullptr},           // leaving key present (for bw compat.)
                  {"result", nullptr}, // true means no captive portal
                  {"User_Agent", nullptr}, // if specified by the vendor
                  {"failure", nullptr}};

            if (input.count("status")) {
                result["http_status_number"] = input.at("status");
            }
            if (input.count("ua")) {
                result["User_Agent"] = input.at("ua");
            }

            if (!!err) {
                logger->info("err: %s", err.as_ooni_error().c_str());
                result["failure"] = err.as_ooni_error();
            } else if (!response) {
                // Should really not happen, no need to be specific
                logger->warn("null response");
                result["failure"] = "unknown_error";
            } else {
                bool unfiltered = false;
                if (input.count("body")) {
                    unfiltered = !!(input.at("body") == response->body);
                } else if (input.count("status")) {
                    // Note: `std::stoul` can throw but the status is set by
                    // us, therefore we do not bother with checking exceptions
                    unfiltered = !!(std::stoul(input.at("status")) ==
                                    response->status_code);
                } else {
                    // Should really not happen, no need to be specific
                    logger->warn("unexpected input format");
                    result["failure"] = "unknown_error";
                }
                logger->info("%s unfiltered: %s", input.at("name").c_str(),
                             unfiltered ? "yes" : "no");
                result["result"] = unfiltered;
            }
            (*entry)["vendor_tests"][input.at("name")] = result;
            done_cb(err);
        };
    };

    std::vector<Continuation<Error>> continuations;
    for (const auto &input : gen_http_inputs()) {
        logger->info("setting up %s", input.at("name").c_str());
        options["http/url"] = input.at("url");
        std::string body;
        http::Headers headers;
        if (input.count("ua")) {
            headers = {{"User-Agent", input.at("ua")}};
        } else {
            headers = constants::COMMON_CLIENT_HEADERS;
        }

        // Note: the following callback uses `[=]`, which means that the
        // options are copied in each run. Thus, each test is getting its
        // right URL to connect to and check.
        continuations.push_back([=](Callback<Error> done_cb) {
            templates::http_request(entry, options, headers, body,
                                    http_cb(input, done_cb), reactor, logger);
        });
    }
    mk::parallel(continuations, all_done_cb, 3);
}

// this is the only test that doesn't follow the pattern of those above.
// this hostname should always resolve to this IP.
void dns_msft_ncsi(Var<Entry> entry, Callback<Error> done_cb, Settings options,
                   Var<Reactor> reactor, Var<Logger> logger) {
    std::string hostname = "dns.msftncsi.com";
    // Note: we're setting the nameserver to empty, which is going to work
    // as long as we're using the `system` DNS resolver. Using another resolver
    // is something we would notice, because parsing an empty endpoint would
    // fail and we would then see the error inside the report.
    std::string nameserver = "";
    templates::dns_query(
          entry, "A", "IN", hostname, nameserver,
          [=](Error err, Var<dns::Message> message) {
              if (!!err) {
                  logger->info("dns_query err: %s",
                               err.as_ooni_error().c_str());
                  done_cb(err);
              } else {
                  bool unfiltered = false;
                  for (const auto &a : message->answers) {
                      // Note: the query is 'A' hence it makes sense that here
                      // we only deal with IPv4 addresses
                      (*entry)["vendor_dns_tests"]["ms_dns_cp"].push_back(
                            a.ipv4);
                      if (a.ipv4 == "131.107.255.255") {
                          unfiltered = true;
                      }
                  }
                  logger->info("MS DNS test unfiltered: %s",
                               (unfiltered ? "yes" : "no"));
                  done_cb(NoError());
              }
          },
          options, reactor, logger);
}

void captiveportal(std::string /*input*/, Settings options,
                   Callback<Var<Entry>> callback, Var<Reactor> reactor,
                   Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    logger->info("starting http_many");
    http_many(entry,
              [=](Error err) {
                  if (err) {
                      logger->warn("http_many error");
                  }
                  logger->info("starting dns_msft_ncsi");
                  dns_msft_ncsi(entry,
                                [=](Error err) {
                                    if (err) {
                                        logger->warn("dns_msft_ncsi error");
                                    }
                                    callback(entry);
                                },
                                options, reactor, logger);
              },
              options, reactor, logger);
}

} // namespace ooni
} // namespace mk
