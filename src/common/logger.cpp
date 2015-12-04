// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/logger.hpp>

#include <stdio.h>

namespace mk {

Logger::Logger() {
    consumer_ = [](const char *s) { fprintf(stderr, "%s\n", s); };
}

void Logger::logv(const char *fmt, va_list ap) {
    if (!consumer_) return;
    int res = vsnprintf(buffer_, sizeof(buffer_), fmt, ap);
    // Once we know that res is non-negative we make it unsigned,
    // which allows the compiler to promote the smaller of res and
    // sizeof (buffer) to the correct size if needed.
    if (res < 0 || (unsigned int)res >= sizeof(buffer_)) return;
    consumer_(buffer_);
}

} // namespace mk
