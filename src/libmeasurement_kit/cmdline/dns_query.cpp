// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/cmdline.hpp>
#include <measurement_kit/dns.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace dns_query {

static const char *kv_usage =
    "usage: measurement_kit dns_query [-v] [-c class] [-t type] domain\n";

int main(const char *, int argc, char **argv) {

    std::string query_class = "IN";
    int ch;
    std::string query_type = "A";
    while ((ch = mkp_getopt(argc, argv, "c:t:v")) != -1) {
        switch (ch) {
        case 'c':
            query_class = mkp_optarg;
            break;
        case 't':
            query_type = mkp_optarg;
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= mkp_optind;
    argv += mkp_optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    std::string domain = argv[0];

    loop_with_initial_event([&query_class, &query_type, &domain]() {
        std::cout << query_class << " " << query_type << "\n";
        dns::query(query_class.data(), query_type.data(), domain,
            [&query_type](Error e, Var<dns::Message> m) {
                if (e) {
                    std::cout << "Error: " << e.code << "\n";
                    break_loop();
                    return;
                }
                for (auto &s : m->answers) {
                    if (query_type == "A") {
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

    return 0;
}

} // namespace dns_query
} // namespace cmdline
} // namespace mk
