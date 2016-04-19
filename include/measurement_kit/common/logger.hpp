// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/funcs.hpp>

#include <functional>
#include <stdarg.h>

namespace mk {

/// Object used to log messages
class Logger : public NonCopyable, public NonMovable {
  public:
    Logger(); ///< Default constructor

    /// Variadic log function
    void logv(const char *, va_list) __attribute__((format(printf, 2, 0)));

    /// Log warning message
    void warn(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (verbose_ >= 0) {
            va_list ap;
            va_start(ap, fmt);
            logv(fmt, ap);
            va_end(ap);
        }
    }

    /// Log info message
    void info(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (verbose_ > 0) {
            va_list ap;
            va_start(ap, fmt);
            logv(fmt, ap);
            va_end(ap);
        }
    }

    /// Log debug message
    void debug(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (verbose_ > 0) {
            va_list ap;
            va_start(ap, fmt);
            logv(fmt, ap);
            va_end(ap);
        }
    }

    void set_verbose(int v) { verbose_ = v; } ///< Set logger verbose

    int get_verbose() { return verbose_; } ///< Get logger verbosity

    /// Set logging function
    void on_log(std::function<void(const char *)> fn) { consumer_ = fn; }

    /// Get global logger
    static Logger *global() {
        static Logger singleton;
        return &singleton;
    }

  private:
    SafelyOverridableFunc<void(const char *)> consumer_;
    int verbose_ = 0;
    char buffer_[32768];
};

inline void warn(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void debug(const char *, ...) __attribute__((format(printf, 1, 2)));
inline void info(const char *, ...) __attribute__((format(printf, 1, 2)));

inline void warn(const char *fmt, ...) {
    auto logger = Logger::global();
    if (logger->get_verbose() >= 0) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void info(const char *fmt, ...) {
    auto logger = Logger::global();
    if (logger->get_verbose() > 0) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void debug(const char *fmt, ...) {
    auto logger = Logger::global();
    if (logger->get_verbose() > 0) {
        va_list ap;
        va_start(ap, fmt);
        logger->logv(fmt, ap);
        va_end(ap);
    }
}

inline void set_verbose(int v) { Logger::global()->set_verbose(v); }

inline void on_log(std::function<void(const char *)> fn) {
    Logger::global()->on_log(fn);
}

} // namespace mk
#endif
