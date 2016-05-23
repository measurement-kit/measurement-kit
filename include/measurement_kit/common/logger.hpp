// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#include <cstdint>
#include <measurement_kit/common/delegate.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/var.hpp>
#include <mutex>
#include <stdarg.h>

// The numbers [0-31] are reserved for verbosity levels. Numbers above 31
// instead are reserved for other, not yet specified semantics.
#define MK_LOG_WARNING 0
#define MK_LOG_INFO 1
#define MK_LOG_DEBUG 2
#define MK_LOG_DEBUG2 3
#define MK_LOG_VERBOSITY_MASK 31

namespace mk {

class Logger : public NonCopyable, public NonMovable {
  public:
    static Var<Logger> make();

    void logv(uint32_t, const char *, va_list)
              __attribute__((format(printf, 3, 0)));
    void log(uint32_t, const char *, ...)
             __attribute__((format(printf, 3, 4)));

    void warn(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void info(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void debug(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    void set_verbosity(uint32_t v) { verbosity_ = (v & MK_LOG_VERBOSITY_MASK); }
    void increase_verbosity();
    uint32_t get_verbosity() { return verbosity_; }

    void on_log(Delegate<uint32_t, const char *> fn) { consumer_ = fn; }

    static Var<Logger> global() {
        static Var<Logger> singleton(new Logger);
        return singleton;
    }

  private:
    Delegate<uint32_t, const char *> consumer_;
    uint32_t verbosity_ = MK_LOG_WARNING;
    char buffer_[32768];
    std::mutex mutex_;

    Logger();
};

void log(uint32_t, const char *, ...) __attribute__((format(printf, 2, 3)));
void warn(const char *, ...) __attribute__((format(printf, 1, 2)));
void debug(const char *, ...) __attribute__((format(printf, 1, 2)));
void info(const char *, ...) __attribute__((format(printf, 1, 2)));

inline void set_verbosity(uint32_t v) { Logger::global()->set_verbosity(v); }
inline void increase_verbosity() { Logger::global()->increase_verbosity(); }
inline uint32_t get_verbosity() { return Logger::global()->get_verbosity(); }

inline void on_log(Delegate<uint32_t, const char *> fn) {
    Logger::global()->on_log(fn);
}

} // namespace mk
#endif
