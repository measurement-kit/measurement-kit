// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example shows usage of `mlabns::query()`
//

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/mlabns.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;

static const char *kv_usage =
    "usage: ./example/mlabns/query [-46v] [-m metro] [-p policy] tool\n";

int main(int argc, char **argv) {

    char ch;
    mlabns::Query query;
    while ((ch = getopt(argc, argv, "46m:p:v")) != -1) {
        switch (ch) {
        case '4':
            query.address_family = "ipv4";
            break;
        case '6':
            query.address_family = "ipv6";
            break;
        case 'm':
            query.metro = optarg;
            break;
        case 'p':
            query.policy = optarg;
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
    std::string tool = argv[0];
    std::cout << "> address_family: " << query.address_family << "\n";
    std::cout << "> metro: " << query.metro << "\n";
    std::cout << "> policy: " << query.policy << "\n";
    std::cout << "> tool: " << tool << "\n";

    loop_with_initial_event([&query, &tool]() {
        mlabns::query(
            tool, [](Error error, mlabns::Reply reply) {
                if (error) {
                    std::cout << "< error: " << (int)error << "\n";
                    break_loop();
                    return;
                }
                std::cout << "< city: " << reply.city << "\n";
                std::cout << "< url: " << reply.url << "\n";
                std::cout << "< ip: [\n";
                for (auto &s : reply.ip) {
                    std::cout << "<  " << s << "\n";
                }
                std::cout << "< ]\n";
                std::cout << "< fqdn: " << reply.fqdn << "\n";
                std::cout << "< site: " << reply.site << "\n";
                std::cout << "< country: " << reply.country << "\n";
                break_loop();
            }, query);
    });
}
