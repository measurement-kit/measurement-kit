// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

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
            increase_verbosity();
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
    loop_with_initial_event([&body, &headers, &method, &url]() {
        http::request(
            {
                {"http/method", method}, {"http/url", url},
            },
            headers,
            body,
            [](Error error, Var<http::Response> response) {
                if (error) {
                    std::cout << "Error: " << (int)error << "\n";
                    break_loop();
                    return;
                }
                std::cout << response->response_line << "\n";
                for (auto &pair : response->headers) {
                    std::cout << pair.first << ": " << pair.second << "\n";
                }
                std::cout << "\n" << response->body << "\n";
                break_loop();
            });
    });
}
