// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/mlabns.hpp>

#include <iostream>

#include <getopt.h>

using namespace mk;

static const char *kv_usage =
    "usage: measurement_kit mlabns [-46v] [-C /path/to/ca/bundle] [-m metro]\n"
    "                              [-p policy] ndt|neubot|ooni|...\n";

static void print_setting(Settings &settings, std::string key) {
    key = "mlabns/" + key;
    std::string value = settings.get<std::string>(key, "");
    std::cout << "> " << key << ": " << value << "\n";
}

int main(int argc, char **argv) {

    int ch;
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

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mk::mlabns::query(
            tool, [=](Error error, mk::mlabns::Reply reply) {
                if (error) {
                    std::cout << "< error: " << (int)error << "\n";
                    reactor->stop();
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
                reactor->stop();
            }, settings);
    });

    return 0;
}
