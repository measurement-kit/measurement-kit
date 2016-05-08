// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#include <functional>
#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/funcs.hpp>
#include <stdarg.h>

namespace mk {

/// Object used to log messages
class Logger : public NonCopyable, public NonMovable {
  public:
    Logger(); ///< Default constructor

    /// Variadic log function
    void logv(const char *, va_list) __attribute__((format(printf, 2, 0)));

    /// Log warning message
    void warn(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    /// Log info message
    void info(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    /// Log debug message
    void debug(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

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

void warn(const char *, ...) __attribute__((format(printf, 1, 2)));
void debug(const char *, ...) __attribute__((format(printf, 1, 2)));
void info(const char *, ...) __attribute__((format(printf, 1, 2)));

inline void set_verbose(int v) { Logger::global()->set_verbose(v); }

inline void on_log(std::function<void(const char *)> fn) {
    Logger::global()->on_log(fn);
}

} // namespace mk
#endif
