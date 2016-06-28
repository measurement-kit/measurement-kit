// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example shows how to use dns::query()
//

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;

static const char *kv_usage =
    "usage: ./example/dns/query [-v] [-c class] [-t type] domain\n";

int main(int argc, char **argv) {

    std::string query_class = "IN";
    char ch;
    std::string query_type = "A";
    while ((ch = getopt(argc, argv, "c:t:v")) != -1) {
        switch (ch) {
        case 'c':
            query_class = optarg;
            break;
        case 't':
            query_type = optarg;
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
    std::string domain = argv[0];

    loop_with_initial_event([&query_class, &query_type, &domain]() {
        std::cout << query_class << " " << query_type << "\n";
        dns::query(query_class.data(), query_type.data(), domain,
            [&query_type](Error e, dns::Message m) {
                if (e) {
                    std::cout << "Error: " << e.code << "\n";
                    break_loop();
                    return;
                }
		for (auto &s : m.answers) {
                    if (query_type == "A"){
                        std::cout << s.ipv4 << "\n";
                    } else if (query_type == "AAAA") {
                        std::cout << s.ipv6 << "\n";
                    } else {
                        std::cout << "Unexpected query type\n";
                    }
                }
                break_loop();
            });
    });
}
