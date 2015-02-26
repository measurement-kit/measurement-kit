/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <stdarg.h>
#include <stdio.h>

#include <ight/common/log.hpp>

class Logger {

    char buffer[32768];
    int verbose_ = 0;
    std::function<void(const char *)> consumer = [](const char *s) {
        fprintf(stderr, "%s\n", s);
    };

    Logger() {}  /* We use singleton pattern */

public:
    static Logger& get() {
        static Logger singleton;
        return singleton;
    }

    void set_verbose(int v) {
        verbose_ = v;
    }

    int verbose() {
        return verbose_;
    }

    void set_logger(std::function<void(const char *)> fn) {
        consumer = fn;
    }

    void log(const char *fmt, va_list ap) {
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
};

void ight_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    Logger::get().log(fmt, ap);
    va_end(ap);
}

void ight_info(const char *fmt, ...) {
    auto logger = Logger::get();
    if (!logger.verbose()) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    logger.log(fmt, ap);
    va_end(ap);
}

void ight_debug(const char *fmt, ...) {
    auto logger = Logger::get();
    if (!logger.verbose()) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    logger.log(fmt, ap);
    va_end(ap);
}

void ight_set_verbose(int v) {
    Logger::get().set_verbose(v);
}

void ight_set_logger(std::function<void(const char *)> fn) {
    Logger::get().set_logger(fn);
}
