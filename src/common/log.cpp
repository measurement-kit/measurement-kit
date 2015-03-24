/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <ight/common/log.hpp>

#include <stdio.h>

using namespace ight::common::log;

Logger::Logger() {
    consumer = [](const char *s) {
        fprintf(stderr, "%s\n", s);
    };
}

void Logger::logv(const char *fmt, va_list ap) {
    int res = vsnprintf(buffer, sizeof (buffer), fmt, ap);
    /*
     * Once we know that res is non-negative we make it unsigned,
     * which allows the compiler to promote the smaller of res and
     * sizeof (buffer) to the correct size if needed.
     */
    if (res < 0 || (unsigned int) res >= sizeof (buffer)) {
        return;
    }
    consumer(buffer);
}
