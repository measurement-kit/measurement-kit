// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/utils.hpp"
#include <getopt.h>
#include <iostream>
#include <measurement_kit/http.hpp>

using namespace mk;

static const char *kv_usage =
        "usage: measurement_kit http_request [-CTv] [-B /ca/bundle/path]\n"
        "       [-b body] [-H 'key: value'] [-m method] [-R max-redirs] url\n";

static bool set_header(http::Headers &headers, const std::string option) {
    auto kv = mk::split<std::vector<std::string>>(option, ":");
    if (kv.size() != 2) {
        std::cout << "invalid header: " << option << "\n";
        return false;
    }
    auto key = kv[0];
    auto value = kv[1];
    // TODO: it would be nice to have {r,l}trim() in common.hpp
    while (value.size() > 0 && value[0] == ' ') {
        value = value.substr(1);
    }
    headers[key] = value;
    return true;
}

int main(int argc, char **argv) {

    SharedPtr<Logger> logger = Logger::global();
    Settings settings;
    std::string body;
    http::Headers headers;
    for (int ch; (ch = getopt(argc, argv, "B:b:CH:m:R:Tv")) != -1;) {
        switch (ch) {
        case 'B':
            settings["net/ca_bundle_path"] = optarg;
            break;
        case 'b':
            body = optarg;
            break;
        case 'C':
            settings["net/allow_ssl23"] = true;
            break;
        case 'H':
            if (!set_header(headers, optarg)) {
                exit(1);
                // NOTREACHED
            }
            break;
        case 'm':
            settings["http/method"] = optarg;
            break;
        case 'R':
            settings["http/max_redirects"] = lexical_cast<int>(optarg);
            break;
        case 'T':
            settings["net/socks5_proxy"] = "127.0.0.1:9050";
            break;
        case 'v':
            logger->increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
            // NOTREACHED
        }
    }
    argc -= optind, argv += optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
        // NOTREACHED
    }
    settings["http/url"] = argv[0];

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        http::request(settings, headers, body,
                [=](Error error, SharedPtr<http::Response> response) {
                    if (error) {
                        std::clog << "Error: " << error << "\n";
                    } else {
                        std::cout << response->body << "\n";
                    }
                    reactor->stop();
                },
                reactor, logger);
    });
}
