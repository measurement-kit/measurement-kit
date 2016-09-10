// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/cmdline.hpp>
#include <measurement_kit/http.hpp>

#include <stdlib.h>

#include <iostream>

namespace mk {
namespace cmdline {
namespace http_request {

static const char *kv_usage =
    "usage: measurement_kit http_request [-v] [-b body] [-m method]\n"
    "                                    [-R max-redirect] url\n";

int main(const char *, int argc, char **argv) {

    Settings settings;
    std::string body;
    int ch;
    while ((ch = getopt(argc, argv, "b:m:R:v")) != -1) {
        switch (ch) {
        case 'b':
            body = optarg;
            break;
        case 'm':
            settings["http/method"] = optarg;
            break;
        case 'R':
            settings["http/max_redirects"] = lexical_cast<int>(optarg);
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
    settings["http/url"] = argv[0];

    http::Headers headers;
    loop_with_initial_event([&]() {
        http::request(
            settings,
            headers,
            body,
            [](Error error, Var<http::Response> response) {
                if (error) {
                    std::cout << "Error: " << error.code << "\n";
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

    return 0;
}

} // namespace http_request
} // namespace cmdline
} // namespace mk
