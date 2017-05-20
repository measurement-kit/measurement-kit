// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/http.hpp>

#include "../../src/libmeasurement_kit/http/response_parser.hpp"

#include <iostream>

#include <getopt.h>

using namespace mk;

static const char *kv_usage =
    "usage: measurement_kit http_request [-v] [-B /ca/bundle/path] [-b body]\n"
    "       [-H 'key: value'] [-m method] [-R max-redirect] url\n";

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

static auto http_read_response_coro() {
    Var<http::ResponseParserNg> parser{new http::ResponseParserNg};
    Var<http::Response> response{new http::Response};
    Var<bool> parsing_complete{new bool{false}};

    parser->on_response([=](http::Response resp) { *response = resp; });
    parser->on_end([=]() { *parsing_complete = true; });
    parser->on_body([=](std::string s) { response->body += s; });

    return [=](net::Buffer data) -> http::Response {
        parser->feed(data);
        if (*parsing_complete == false) {
            throw YieldError(); /* Interrupt here the coroutine */
        }
        return *response;
    };
}

static auto add_request_coro(Var<http::Request> request) {
    return [request = std::move(request)](http::Response response) {
        response.request = request;
        return response;
    };
}

static auto print_response_and_exit_coro() {
    return [](http::Response response) {
        std::cout << response.response_line << "\n";
        for (auto &pair : response.headers) {
            std::cout << pair.first << ": " << pair.second << "\n";
        }
        std::cout << "\n" << response.body << "\n";
        break_loop();
    };
}

int main(int argc, char **argv) {

    Settings settings;
    std::string body;
    http::Headers headers;
    int ch;
    while ((ch = getopt(argc, argv, "B:b:H:m:R:v")) != -1) {
        switch (ch) {
        case 'B':
            settings["net/ca_bundle_path"] = optarg;
            break;
        case 'b':
            body = optarg;
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
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    settings["http/url"] = argv[0];

    loop_with_initial_event([=]() {
        http::request_connect(settings, [=](Error e, Var<net::Transport> t) {
            if (e) {
                throw e;
            }
            http::request_send(
                t, settings, headers, body, [=](Error e, Var<http::Request> r) {
                    if (e) {
                        throw e;
                    }
                    mk::warn("Request sent... waiting for response...");
                    t->on_data(fcompose(
                        http_read_response_coro(),
                        add_request_coro(std::move(r)),
                        print_response_and_exit_coro()
                    ));
                    t->on_error([](Error e) {
                        std::cout << "Error: " << e.explain() << "\n";
                        throw e;
                    });
                });
        });
    });

    return 0;
}
