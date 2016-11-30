// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"

#include <stdio.h>
#include <string.h>

namespace mk {
namespace cmdline {

static int usage(int retval, FILE *filep, const char *progname);

static int help(const char *progname, int, char **) {
    return usage(0, stdout, progname);
}

#define XX(nn) { #nn, nn::main }

static const struct {
    const char *mod_name;
    int (*mod_func)(const char *, int, char **);
} modules[] = {
    XX(dns_injection),
    XX(dns_query),
    XX(http_invalid_request_line),
    XX(http_request),
    XX(mlabns),
    XX(multi_ndt),
    XX(ndt),
    XX(net_connect),
    XX(oonireport),
    XX(ooniresources),
    XX(tcp_connect),
    XX(web_connectivity),
    {"--help", help},
    {nullptr, nullptr},
};

#undef XX

static int usage(int retval, FILE *filep, const char *progname) {
    fprintf(filep, "usage: %s [module] [options] [arguments]\n", progname);
    fprintf(filep, "available modules:\n");
    for (auto p = &modules[0]; p->mod_name != nullptr; ++p) {
        if (p->mod_name[0] != '-') {
            fprintf(filep, "- %s\n", p->mod_name);
        }
    }
    fprintf(filep, "run `%s <module> --help' for more help\n", progname);
    return retval;
}

int main(const char *progname, int argc, char **argv) {
    --argc, ++argv;
    if (argc == 0) {
        return usage(1, stderr, progname);
    }
    for (auto p = &modules[0]; p->mod_name != nullptr; ++p) {
        if (strcmp(argv[0], p->mod_name) == 0) {
            return p->mod_func(progname, argc, argv);
        }
    }
    fprintf(stderr, "%s: module not found: %s\n", progname, argv[0]);
    return usage(1, stderr, progname);
}

} // namespace cmdline
} // namespace mk
