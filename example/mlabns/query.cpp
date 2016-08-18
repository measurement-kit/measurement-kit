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
    "usage: ./example/mlabns/query [-46v] [-C /path/to/ca/bundle] [-m metro]\n"
    "                              [-p policy] ndt|neubot|ooni|...\n";

static void print_setting(Settings &settings, std::string key) {
    key = "mlabns/" + key;
    std::string value = settings.get<std::string>(key, "");
    std::cout << "> " << key << ": " << value << "\n";
}

int main(int argc, char **argv) {

    char ch;
    Settings settings;
    while ((ch = getopt(argc, argv, "46C:m:p:v")) != -1) {
        switch (ch) {
        case '4':
            settings["mlabns/address_family"] = "ipv4";
            break;
        case '6':
            settings["mlabns/address_family"] = "ipv6";
            break;
        case 'C':
            settings["net/ca_bundle_path"] = optarg;
            break;
        case 'm':
            settings["mlabns/metro"] = optarg;
            break;
        case 'p':
            settings["mlabns/policy"] = optarg;
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
    print_setting(settings, "address_family");
    print_setting(settings, "metro");
    print_setting(settings, "policy");
    std::cout << "> tool: " << tool << "\n";

    loop_with_initial_event([=]() {
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
            }, settings);
    });
}
