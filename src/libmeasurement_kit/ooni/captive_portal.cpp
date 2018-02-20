// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/common/locked.hpp"
#include "src/libmeasurement_kit/common/parallel.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

typedef std::map<std::string, std::string> input_t; // Syntactic sugar

static const std::vector<input_t> &gen_http_inputs() {
    // Implementation note: the vector must be static and outside of the lambda
    // to prevent several clang warning re: return a stack allocated object.
    // This seems weird to me, but perhaps it's just that I do not fully grok
    // declaring a static variable inside a lambda (maybe it's not so weird
    // if one thinks that a lambda is a class).
    static std::vector<input_t> is;
    locked_global([]() {
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
    });
    return is;
}

static void http_many(SharedPtr<Entry> entry, Callback<Error> all_done_cb,
        Settings options, SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger) {

    auto http_cb = [=](const input_t &input, Callback<Error> done_cb) {
        return [=](Error err, SharedPtr<http::Response> response) {
            // result: true means unfiltered
            Entry result = {{"URL", input.at("url")},
                    {"http_status_number",
                            nullptr}, // expected status code, if there is one
                    {"http_status_summary",
                            nullptr},    // leaving key present (for bw compat.)
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
                logger->info("err: %s", err.what());
                result["failure"] = err.reason;
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

static void dns_msft_ncsi(SharedPtr<Entry> entry, Callback<Error> done_cb,
        Settings options, SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger) {
    std::string hostname = "dns.msftncsi.com";
    // Note: we're setting the nameserver to empty, which is going to work
    // as long as we're using the `system` DNS resolver. Using another resolver
    // is something we would notice, because parsing an empty endpoint would
    // fail and we would then see the error inside the report.
    std::string nameserver = "";
    templates::dns_query(entry, "A", "IN", hostname, nameserver,
            [=](Error err, SharedPtr<dns::Message> message) {
                if (!!err) {
                    logger->info("dns_query err: %s", err.what());
                    done_cb(err);
                } else {
                    bool unfiltered = false;
                    for (const auto &a : message->answers) {
                        if (a.type != dns::MK_DNS_TYPE_A) {
                            // Skip the CNAME associated to that A address
                            continue;
                        }
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

// DNS A lookups against some random hostnames unlikely to point at anything.
static void dns_random_hostnames(size_t count, size_t length,
        SharedPtr<Entry> entry, Callback<Error> done_cb, Settings options,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    // if any random domains resolve, change to false
    // (true means unfiltered)
    (*entry)["vendor_dns_tests"]["google_dns_cp"]["result"] = true;
    (*entry)["vendor_dns_tests"]["google_dns_cp"]["addresses"] = Entry::array();
    SharedPtr<size_t> names_tested(new size_t(0));

    auto dns_cb = [=](std::string hostname) {
        return [=](Error err, SharedPtr<dns::Message> message) {
            if (!!err) {
                if ((err == mk::dns::NotExistError()) ||
                        (err == mk::dns::
                                        HostOrServiceNotProvidedOrNotKnownError())) {
                    logger->info("%s: NXDOMAIN", hostname.c_str());
                } else {
                    logger->info("%s", err.what());
                    logger->info("captive_portal: dns error for %s",
                            hostname.c_str());
                }
            } else {
                for (auto answer : message->answers) {
                    if ((answer.ipv4 != "")) {
                        (*entry)["vendor_dns_tests"]["google_dns_cp"]
                                ["result"] = false;
                        logger->info("%s: %s", hostname.c_str(),
                                answer.ipv4.c_str());
                        (*entry)["vendor_dns_tests"]["google_dns_cp"]
                                ["addresses"]
                                        .push_back(answer.ipv4.c_str());
                    } else {
                        // XXX not sure how to treat blank answers
                        logger->info("%s: blank", hostname.c_str());
                    }
                }
            }
            *names_tested += 1;
            assert(*names_tested <= count);
            if (count == *names_tested) {
                if ((*entry)["vendor_dns_tests"]["google_dns_cp"]["addresses"]
                                .empty()) {
                    logger->info(
                            "all returned NXDOMAIN; we call this unfiltered");
                } else {
                    logger->info("unexpectedly resolved something random: "
                                 "evidence of filtering");
                }
                done_cb(NoError());
                return;
            }
        };
    };

    std::vector<std::string> hostnames = {};
    for (size_t i = 0; i < count; i++) {
        hostnames.push_back(
                mk::random_str_lower_alpha(length) + mk::random_tld());
    }

    for (auto const &hostname : hostnames) {
        // Note: we're passing in an empty nameserver, which rests on the
        // assumption that we're using the `system` DNS resolver.
        constexpr const char *nameserver = "";
        templates::dns_query(entry, "A", "IN", hostname, nameserver,
                dns_cb(hostname), options, reactor, logger);
    }
}

void captiveportal(std::string /*input*/, Settings options,
        Callback<SharedPtr<Entry>> callback, SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger) {
    SharedPtr<Entry> entry(new Entry);
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
                            logger->info("starting random hostnames");
                            dns_random_hostnames(3, 10, entry,
                                    [=](Error err) {
                                        if (err) {
                                            logger->warn("dns_msft_ncsi error");
                                        }
                                        callback(entry);
                                    },
                                    options, reactor, logger);
                        },
                        options, reactor, logger);
            },
            options, reactor, logger);
}

} // namespace ooni
} // namespace mk
