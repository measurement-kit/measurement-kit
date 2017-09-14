// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#include <cstdint>
#include <measurement_kit/common/aaa_base.hpp>
#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <stdarg.h>

// The numbers [0-31] are reserved for verbosity levels.
#define MK_LOG_WARNING 0
#define MK_LOG_INFO 1
#define MK_LOG_DEBUG 2
#define MK_LOG_DEBUG2 3
#define MK_LOG_VERBOSITY_MASK 31

// The number above 31 have different semantics:
#define MK_LOG_EVENT 32 // Information encoded as JSON

namespace mk {

class Logger {
  public:
    static SharedPtr<Logger> make();

    static SharedPtr<Logger> global();

    virtual void logv(uint32_t, const char *, va_list)
        __attribute__((format(printf, 3, 0))) = 0;

    virtual void log(uint32_t, const char *, ...)
        __attribute__((format(printf, 3, 4))) = 0;

    virtual void warn(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    virtual void info(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    virtual void debug(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    virtual void debug2(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    virtual void set_verbosity(uint32_t v) = 0;

    virtual void increase_verbosity() = 0;

    virtual uint32_t get_verbosity() = 0;

    virtual void on_log(Callback<uint32_t, const char *> &&fn) = 0;

    virtual void on_eof(Callback<> &&fn) = 0;

    virtual void on_event(Callback<const char *> &&fn) = 0;

    virtual void on_progress(Callback<double, const char *> &&fn) = 0;

    virtual void set_logfile(std::string fpath) = 0;

    virtual void progress(double, const char *) = 0;

    virtual void progress_relative(double, const char *) = 0;

    virtual void set_progress_offset(double offset) = 0;

    virtual void set_progress_scale(double scale) = 0;

    virtual ~Logger();
};

void log(uint32_t, const char *, ...) __attribute__((format(printf, 2, 3)));

void warn(const char *, ...) __attribute__((format(printf, 1, 2)));

void info(const char *, ...) __attribute__((format(printf, 1, 2)));

void debug(const char *, ...) __attribute__((format(printf, 1, 2)));

void debug2(const char *, ...) __attribute__((format(printf, 1, 2)));

void set_verbosity(uint32_t v);

void increase_verbosity();

uint32_t get_verbosity();

void on_log(Callback<uint32_t, const char *> &&fn);

void set_logfile(std::string path);

} // namespace mk
#endif
