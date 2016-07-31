// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <measurement_kit/scripting.hpp>
#include <string>
#include <unistd.h>

static const char *kv_usage = "usage: ./example/scripting/chai [-v] path\n";

int main(int argc, char **argv) {

    char ch;
    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
        case 'v':
            mk::increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }

    mk::scripting::chai(argv[0]);
}
