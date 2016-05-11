// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/logger.hpp>
#include <stdio.h>

namespace mk {

/*static*/ Var<Logger> Logger::make() { return Var<Logger>(new Logger); }

Logger::Logger() {
    consumer_ = [](uint32_t level, const char *s) {
        fprintf(stderr, "<%d> %s\n", level, s);
    };
}

void Logger::logv(uint32_t level, const char *fmt, va_list ap) {
    if (!consumer_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int res = vsnprintf(buffer_, sizeof(buffer_), fmt, ap);
    // Once we know that res is non-negative we make it unsigned,
    // which allows the compiler to promote the smaller of res and
    // sizeof (buffer) to the correct size if needed.
    if (res < 0 || (unsigned int)res >= sizeof(buffer_)) {
        return;
    }
    consumer_(level, buffer_);
}

#define XX(_logger_, _level_)                                                  \
    do {                                                                       \
        uint32_t real_level = (_level_) & MK_LOG_VERBOSITY_MASK;               \
        if (real_level <= _logger_->get_verbosity()) {                         \
            va_list ap;                                                        \
            va_start(ap, fmt);                                                 \
            _logger_->logv(_level_, fmt, ap);                                  \
            va_end(ap);                                                        \
        }                                                                      \
    } while (0)

void Logger::log(uint32_t level, const char *fmt, ...) { XX(this, level); }
void Logger::warn(const char *fmt, ...) { XX(this, MK_LOG_WARNING); }
void Logger::info(const char *fmt, ...) { XX(this, MK_LOG_INFO); }
void Logger::debug(const char *fmt, ...) { XX(this, MK_LOG_DEBUG); }

void log(uint32_t level, const char *fmt, ...) { XX(Logger::global(), level); }
void warn(const char *fmt, ...) { XX(Logger::global(), MK_LOG_WARNING); }
void info(const char *fmt, ...) { XX(Logger::global(), MK_LOG_INFO); }
void debug(const char *fmt, ...) { XX(Logger::global(), MK_LOG_DEBUG); }

#undef XX

void Logger::increase_verbosity() {
    if (verbosity_ < MK_LOG_VERBOSITY_MASK) {
        ++verbosity_;
    }
}

} // namespace mk
