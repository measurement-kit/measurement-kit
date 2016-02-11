// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/dns/response.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "src/dns/getaddrinfo.hpp"

using namespace mk;

#define USAGE "usage: %s [-v] domain [...]\n"

int main(int argc, char **argv) {
    const char *progname = argv[0];
    int ch;

    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
        case 'v':
            mk::set_verbose(1);
            break;
        default:
            fprintf(stderr, USAGE, progname);
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    std::vector<std::string> domains;
    while (argc > 0) {
        std::cout << "- " << argv[0] << ":\n";
        auto response = dns::getaddrinfo_query("IN", "A", argv[0]);
        if (!response) {
            std::cout << "\n";
        } else {
            for (auto &addr : response.as_value().results) {
                std::cout << "    - " << addr << "\n";
            }
        }
        --argc, ++argv;
    }
}
