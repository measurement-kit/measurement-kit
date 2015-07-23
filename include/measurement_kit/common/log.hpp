// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_LOG_HPP
#define MEASUREMENT_KIT_COMMON_LOG_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <functional>
#include <stdarg.h>

namespace measurement_kit {
namespace common {

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

} // namespace common

inline void warn(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void debug(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void info(const char *, ...) __attribute__((format(printf, 1, 2)));

inline void warn(const char *fmt, ...) {
    auto logger = common::DefaultLogger::get();
    va_list ap;
    va_start(ap, fmt);
    logger->logv(fmt, ap);
    va_end(ap);
}

inline void info(const char *fmt, ...) {
    auto logger = common::DefaultLogger::get();
    if (logger->is_verbose()) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void debug(const char *fmt, ...) {
    auto logger = common::DefaultLogger::get();
    if (logger->is_verbose()) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void set_verbose(int v) {
    common::DefaultLogger::get()->set_verbose(v);
}

inline void set_logger(std::function<void(const char *)> fn) {
    common::DefaultLogger::get()->set_logger(fn);
}

} // namespace measurement_kit
#endif
