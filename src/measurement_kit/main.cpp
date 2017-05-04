// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../measurement_kit/cmdline.hpp"

#include <cstdio>
#include <cstring>

static const struct {
    const char *mod_name;
    int (*mod_func)(std::list<Callback<BaseTest &>> &, int, char **);
} kv_modules[] = {
#define XX(nn) {#nn, nn::main},
    MK_CMDLINE_SUBCOMMANDS //
    {nullptr, nullptr},
#undef XX
};

static OptionSpec kv_specs[] = {
    {'c', "collector", true, "URL", "Set custom collector base URL"},
    {'g', "no-geoip", false, nullptr, "Disable geoip lookup"},
    {'l', "logfile", true, "PATH", "Set logfile PATH"},
    {'N', "no-json", false, nullptr, "Disable writing to disk"},
    {'n', "no-collector", false, nullptr, "Disable writing to collect"},
    {'o', "reportfile", true, "PATH", "Set custom report file PATH"},
    {'s', "list", false, nullptr, "List available nettests"},
    {'v', "verbose", false, nullptr, "Increase verbosity"},
    {256, "help", false, nullptr, "Display this help and exit"},
    {257, "version", false, nullptr, "Display version number and exit"},
    {0, nullptr, 0, 0, nullptr}
};

#define USAGE                                                                  \
    "usage: measurement_kit [global_options] [nettest] [nettest_options...]\n"

static int usage(int retval, FILE *fp) {
    fprintf(fp, "%s\n", USAGE);
    fprintf(fp, "%s", as_available_options_string(kv_specs).c_str());
    fprintf(fp, "Try `measurement_kit <nettest> --help' for specific help\n");
    return retval;
}

int main(int argc, char **argv) {
    std::list<Callback<BaseTest &>> initializers;
    std::vector<option> long_options = as_long_options(kv_specs);
    std::string stropt = as_getopt_string(kv_specs);
    auto do_list = false;

    for (int ch; (ch = getopt_long(argc, argv, stropt.c_str(),
                                   long_options.data(), nullptr)) != -1;) {
        switch (ch) {
        case 'c':
            [&]() {
                std::string collector = optarg;
                initializers.push_back([=](BaseTest &test) {
                    test.set_options("collector_base_url", collector);
                });
            }();
            break;
        case 'g':
            initializers.push_back([](BaseTest &test) {
                test.set_options("save_real_probe_asn", "0");
                test.set_options("save_real_probe_cc", "0");
            });
            break;
        case 'l':
            [&]() {
                std::string logfile = optarg;
                initializers.push_back(
                    [=](BaseTest &test) { test.set_error_filepath(logfile); });
            }();
            break;
        case 'N':
            initializers.push_back([](BaseTest &test) {
                test.set_options("no_file_report", "1");
            });
            break;
        case 'n':
            initializers.push_back(
                [](BaseTest &test) { test.set_options("no_collector", "1"); });
            break;
        case 'o':
            [&]() {
                std::string reportfile = optarg;
                initializers.push_back([=](BaseTest &test) {
                    test.set_output_filepath(reportfile);
                });
            }();
            break;
        case 's':
            do_list = true;
            break;
        case 'v':
            initializers.push_back(
                [](BaseTest &test) { test.increase_verbosity(); });
            break;
        case 256:
            return usage(0, stdout);
        case 257:
            printf("measurement-kit version: %s\n", mk_version());
            printf("libevent version: %s\n", mk_libevent_version());
            printf("OpenSSL version: %s\n", mk_openssl_version());
            exit(0);
            /* NOTREACHED */
        default:
            return usage(1, stderr);
        }
    }
    argc -= optind, argv += optind;

    if (do_list) {
        printf("Available nettests:\n");
        for (auto p = &kv_modules[0]; p->mod_name != nullptr; ++p) {
            printf("- %s\n", p->mod_name);
        }
        exit(0);
        /* NOTREACHED */
    }
    if (argc <= 0) {
        return usage(1, stderr);
    }

    /*
     * Allow to call getopt() again.
     *
     * Non portable. Assume it's either GNU or BSD. We can do better in
     * configure checking for the proper way to reset options.
     */
#ifdef __GLIBC__
    optind = 0;
#elif (defined __APPLE__ || defined __FreeBSD__ || defined __OpenBSD__ ||      \
       defined __NetBSD__ || defined __DragonFly__)
    optreset = 1, optind = 1;
#else
#error "Don't know how to reset getopt() on your system"
#endif
    for (auto p = &kv_modules[0]; p->mod_name != nullptr; ++p) {
        if (strcmp(argv[0], p->mod_name) == 0) {
            return p->mod_func(initializers, argc, argv);
        }
    }
    fprintf(stderr, "measurement_kit: nettest not found: %s\n", argv[0]);
    return 1;
}
