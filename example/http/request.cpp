// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example shows how to use http::request()
//

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;

static const char *kv_usage =
    "usage: ./example/http/request [-v] [-b body] [-m method] url\n";

int main(int argc, char **argv) {

    std::string body;
    char ch;
    std::string method = "GET";
    while ((ch = getopt(argc, argv, "b:m:v")) != -1) {
        switch (ch) {
        case 'b':
            body = optarg;
            break;
        case 'm':
            method = optarg;
            break;
        case 'v':
            set_verbose(1);
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    std::string url = argv[0];

    http::Headers headers;
    Poller *poller = Poller::global();
    poller->call_soon([&body, &headers, &method, &poller, &url]() {
        http::request(
            {
                {"http/method", method}, {"http/url", url},
            },
            [&poller](Error error, http::Response response) {
                if (error) {
                    std::cout << "Error: " << (int)error << "\n";
                    poller->break_loop();
                    return;
                }
                std::cout << response.response_line << "\n";
                for (auto &pair : response.headers) {
                    std::cout << pair.first << ": " << pair.second << "\n";
                }
                std::cout << "\n" << response.body << "\n";
                poller->break_loop();
            },
            headers, body);
    });
    poller->loop();
}
