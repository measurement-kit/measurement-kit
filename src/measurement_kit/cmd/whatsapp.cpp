// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace whatsapp {

#define USAGE                                       \
    "usage: measurement_kit [options] whatsapp\n"   \

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::WhatsappTest test;
    int ch;
    while ((ch = getopt(argc, argv, "")) != -1) {
        switch (ch) {
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc != 0) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }

    common_init(initializers, test).run();
    return 0;
}

} // namespace whatsapp
