/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_LOG_HPP
# define IGHT_COMMON_LOG_HPP

#include <ight/common/constraints.hpp>
#include <ight/common/pointer.hpp>

#include <functional>
#include <stdarg.h>

namespace ight {
namespace common {
namespace log {

using namespace ight::common::constraints;
using namespace ight::common::pointer;

class Logger : public NonCopyable, public NonMovable {

  private:
    std::function<void(const char *)> consumer;
    int verbose = 0;
    char buffer[32768];

  public:
    Logger();

    void logv(const char *, va_list) __attribute__((format(printf, 2, 0)));

    void warn(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        va_list ap;
        va_start(ap, fmt);
        logv(fmt, ap);
        va_end(ap);
    }

    void info(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (verbose) {
            va_list ap;
            va_start(ap, fmt);
            logv(fmt, ap);
            va_end(ap);
        }
    }

    void debug(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (verbose) {
            va_list ap;
            va_start(ap, fmt);
            logv(fmt, ap);
            va_end(ap);
        }
    }

    void set_verbose(int v) {
        verbose = v;
    }

    int is_verbose() {
        return verbose;
    }

    void set_logger(std::function<void(const char *)> fn) {
        consumer = fn;
    }
};

inline SharedPointer<Logger> make_logger() {
    return SharedPointer<Logger>{new Logger};
}

struct DefaultLogger {
    static SharedPointer<Logger> get() {
        static SharedPointer<Logger> singleton = make_logger();
        return singleton;
    }
};

}}}

inline void ight_warn(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void ight_debug(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void ight_info(const char *, ...) __attribute__((format(printf, 1, 2)));

inline void ight_warn(const char *fmt, ...) {
    auto logger = ight::common::log::DefaultLogger::get();
    va_list ap;
    va_start(ap, fmt);
    logger->logv(fmt, ap);
    va_end(ap);
}

inline void ight_info(const char *fmt, ...) {
    auto logger = ight::common::log::DefaultLogger::get();
    if (logger->is_verbose()) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void ight_debug(const char *fmt, ...) {
    auto logger = ight::common::log::DefaultLogger::get();
    if (logger->is_verbose()) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void ight_set_verbose(int v) {
    ight::common::log::DefaultLogger::get()->set_verbose(v);
}

inline void ight_set_logger(std::function<void(const char *)> fn) {
    ight::common::log::DefaultLogger::get()->set_logger(fn);
}

#endif
