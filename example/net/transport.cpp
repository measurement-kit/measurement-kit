// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/net.hpp>
#include <regex>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::net;

static const char *kv_usage =
        "usage: ./example/net/transport [-Sv] [-P address:port] url\n";

static void print_line(std::string line) {
    std::string s;
    for (auto chr : line) {
        if (chr == '\r' || chr == '\n') {
            break;
        }
        s += chr;
    }
    debug("< %s", s.c_str());
}

int main(int argc, char **argv) {

    Settings settings;
    char ch;
    while ((ch = getopt(argc, argv, "P:Sv")) != -1) {
        switch (ch) {
        case 'P':
            settings["net/socks5_proxy"] = optarg;
            break;
        case 'S':
            settings["net/ssl"] = true;
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
    http::Url url = http::parse_url(argv[0]);

    loop_with_initial_event([&]() {
        connect(url.address, url.port, [&](Error error, Var<Transport> tx) {
            if (error) {
                debug("* error: %d", (int)error);
                break_loop();
                return;
            }
            Var<Buffer> incoming(new Buffer);
            Var<bool> reading_meta(new bool(true));
            tx->set_timeout(10);
            tx->write("GET " + url.pathquery + " HTTP/1.0\r\n");
            debug("> GET %s HTTP/1.0", url.pathquery.c_str());
            tx->write("Accept: */*\r\n");
            debug("> Accept: */*");
            tx->write("Connection: close\r\n");
            debug("> Connection: close");
            tx->write("Host: " + url.address + "\r\n");
            debug("> Host: %s", url.address.c_str());
            tx->write("\r\n");
            debug(">");
            tx->on_error([=](Error error) {
                if (error != EofError()) {
                    debug("* error: %d", (int)error);
                } else {
                    debug("* EOF");
                }
                tx->close([]() {
                    break_loop();
                });
            });
            tx->on_data([=](Buffer data) {
                *incoming << data;
                while (*reading_meta) {
                    ErrorOr<std::string> line = incoming->readline(1024);
                    if (!line) {
                        tx->emit_error(line.as_error());
                        return;
                    }
                    if (*line == "") {
                        return;
                    }
                    print_line(*line);
                    if (*line == "\r\n" || *line == "\n") {
                        *reading_meta = false;
                        break;
                    }
                }
                debug("<+%llu-bytes", (unsigned long long)incoming->length());
            });
        }, settings);
    });
}
