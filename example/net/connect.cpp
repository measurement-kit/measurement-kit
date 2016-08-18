// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::net;

static const char *kv_usage =
        "usage: ./example/net/connect [-Tv] [-p port] [-t timeout] domain\n";

int main(int argc, char **argv) {

    int port = 80;
    Settings settings;
    char ch;
    while ((ch = getopt(argc, argv, "p:Tt:v")) != -1) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(optarg);
            break;
        case 'T':
            settings["net/socks5_proxy"] = "127.0.0.1:9050";
            break;
        case 't':
            settings["net/timeout"] = lexical_cast<double>(optarg);
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
    std::string domain = argv[0];

    loop_with_initial_event([&domain, &port, &settings]() {
        connect(domain, port, [](Error err, Var<Transport> txp) {
            std::cout << "Overall connect result: " << err.code << "\n";
            Var<ConnectResult> cr = err.context.as<ConnectResult>();
            if (!cr) {
                std::cout << "No connection information\n";
            } else {
                std::cout << "input was valid ipv4: " <<
                        cr->resolve_result.inet_pton_ipv4 << "\n";
                std::cout << "input was valid ipv6: " <<
                        cr->resolve_result.inet_pton_ipv4 << "\n";
                std::cout << "ipv4 resolve error: " <<
                        (int) cr->resolve_result.ipv4_err << "\n";
                std::cout << "ipv6 resolve error: " <<
                        (int) cr->resolve_result.ipv6_err << "\n";
                std::cout << "list of addresses returned:\n";
                for (auto addr : cr->resolve_result.addresses) {
                    std::cout << "    - " << addr << "\n";
                }
                std::cout << "errors returned by the various connects:\n";
                for (auto e : cr->connect_result) {
                    std::cout << "    - " << e.code << "\n";
                }
                std::cout << "bufferevent address used internally: "
                        << cr->connected_bev << "\n";
            }
            if (txp) {
                txp->close([]() {
                    break_loop();
                });
            } else {
                break_loop();
            }
        }, settings);
    });
}
