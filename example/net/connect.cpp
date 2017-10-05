// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <iostream>
#include <measurement_kit/net.hpp>
#include <unistd.h>

using namespace mk::net;
using namespace mk;

static const char *kv_usage =
        "usage: ./example/net/connect [-STv] [-p port] [-t timeout] domain\n";

int main(int argc, char **argv) {

    SharedPtr<Logger> logger = Logger::make();
    int port = 80;
    Settings settings;
    for (int ch; (ch = getopt(argc, argv, "p:STt:v")) != -1;) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(optarg);
            break;
        case 'S':
            settings["net/ssl"] = true;
            break;
        case 'T':
            settings["net/socks5_proxy"] = "127.0.0.1:9050";
            break;
        case 't':
            settings["net/timeout"] = lexical_cast<double>(optarg);
            break;
        case 'v':
            logger->increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
            /* NOTREACHED */
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
        net::connect(domain, port,
                [=](Error err, SharedPtr<net::Transport> txp) {
                    std::cout << "Overall connect result: " << err << "\n";
                    auto rr = txp->dns_result();
                    std::cout << "- valid ipv4? " << rr.inet_pton_ipv4 << "\n";
                    std::cout << "- valid ipv6? " << rr.inet_pton_ipv6 << "\n";
                    std::cout << "- ipv4 dns query: " << rr.ipv4_err << "\n";
                    std::cout << "- ipv6 dns query: " << rr.ipv6_err << "\n";
                    std::cout << "- list of addresses returned:\n";
                    for (auto addr : rr.addresses) {
                        std::cout << "  - " << addr << "\n";
                    }
                    std::cout << "errors returned by the various connects:\n";
                    for (auto e : txp->connect_errors()) {
                        std::cout << "    - " << e << "\n";
                    }
                    txp->close(nullptr);
                },
                settings, reactor, logger);
    });
}
