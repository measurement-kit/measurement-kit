// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "../cmdline.hpp"

namespace whatsapp {

#define USAGE "usage: measurement_kit [options] whatsapp [-e]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::WhatsappTest test;
    for (int ch; (ch = getopt(argc, argv, "e")) != -1;) {
        switch (ch) {
        case 'e':
            test.set_option("all_endpoints", (int64_t)1);
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            /* NOTREACHED */
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
