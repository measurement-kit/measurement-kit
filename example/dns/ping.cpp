// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>

#include <iostream>

#include <unistd.h>

static const char *kv_usage =
      "usage: ./example/dns/ping [-v] [-c class] [-e engine] [-i interval]\n"
      "                          [-m max_runtime] [-N nameserver]\n"
      "                          [-r retries] [-T timeout] [-t type] domain\n";

using namespace mk;

int main(int argc, char **argv) {
    Settings settings;
    settings["dns/engine"] = "libevent";
    settings["dns/retries"] = 1;
    settings["dns/timeout"] = 1.0;
    std::string query_class = "IN";
    std::string query_type = "A";
    double interval = 1.0;
    Maybe<double> max_runtime = 10.0;
    Var<Reactor> reactor = Reactor::make();
    Var<Logger> logger = Logger::make();
    for (int ch; (ch = getopt(argc, argv, "c:e:i:m:N:r:T:t:v")) != -1;) {
        switch (ch) {
        case 'c':
            query_class = optarg;
            break;
        case 'e':
            settings["dns/engine"] = optarg;
            break;
        case 'i':
            interval = lexical_cast<double>(optarg);
            break;
        case 'm':
            *max_runtime = lexical_cast<double>(optarg);
            break;
        case 'N':
            settings["dns/nameserver"] = optarg;
            break;
        case 'r':
            settings["dns/retries"] = optarg;
            break;
        case 'T':
            settings["dns/timeout"] = optarg;
            break;
        case 't':
            query_type = optarg;
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
        /* NOTREACHED */
    }
    std::string domain = argv[0];
    *max_runtime += mk::time_now(); // Make it absolute
    reactor->run_with_initial_event([&]() {
        logger->info("Entering into the loop");
        dns::ping_nameserver(query_class, query_type, domain, interval,
                             max_runtime, settings, reactor, logger,
                             [=](Error err, Var<dns::Message> m) {
                                 std::cout << query_class << " " << query_type;
                                 if (err) {
                                     std::cout << " " << err.explain() << "\n";
                                     return;
                                 }
                                 std::cout << " " << m->rtt;
                                 for (auto &s : m->answers) {
                                     if (query_type == "A") {
                                         std::cout << " " << s.ipv4;
                                     } else if (query_type == "AAAA") {
                                         std::cout << " " << s.ipv6;
                                     } else {
                                         std::cout << "Unexpected query type\n";
                                     }
                                 }
                                 std::cout << "\n";
                             },
                             [=](Error error) {
                                 logger->info("Exiting from the loop: %d",
                                              error.code);
                                 reactor->stop();
                             });
    });
}
