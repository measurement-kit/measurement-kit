// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/net.hpp>

#include <iostream>

#include <unistd.h>

using namespace mk::net;
using namespace mk;

static const char *kv_usage =
        "usage: ./example/net/connect [-Tv] [-p port] [-t timeout] domain\n";

int main(int argc, char **argv) {

    int port = 80;
    Settings settings;
    int ch;
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

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect(domain, port, [=](Error err, SharedPtr<Transport> txp) {
            std::cout << "Overall connect result: " << err << "\n";
            auto resolve_result = txp->dns_result();
            std::cout << "input was valid ipv4: " <<
                    resolve_result.inet_pton_ipv4 << "\n";
            std::cout << "input was valid ipv6: " <<
                    resolve_result.inet_pton_ipv6 << "\n";
            std::cout << "ipv4 resolve error: " <<
                    resolve_result.ipv4_err << "\n";
            std::cout << "ipv6 resolve error: " <<
                    resolve_result.ipv6_err << "\n";
            std::cout << "list of addresses returned:\n";
            for (auto addr : resolve_result.addresses) {
                std::cout << "    - " << addr << "\n";
            }
            std::cout << "errors returned by the various connects:\n";
            for (auto e : txp->connect_errors()) {
                std::cout << "    - " << e << "\n";
            }
            txp->close([=]() { reactor->stop(); });
        }, settings);
    });

    return 0;
}
