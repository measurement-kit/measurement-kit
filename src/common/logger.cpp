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

#define XX(_logger_, _level_)                                                  \
    if (_logger_->get_verbose() >= _level_) {                                  \
        va_list ap;                                                            \
        va_start(ap, fmt);                                                     \
        _logger_->logv(fmt, ap);                                               \
        va_end(ap);                                                            \
    }

void Logger::warn(const char *fmt, ...) { XX(this, 0); }
void Logger::info(const char *fmt, ...) { XX(this, 1); }
void Logger::debug(const char *fmt, ...) { XX(this, 1); }

void warn(const char *fmt, ...) { XX(Logger::global(), 0); }
void info(const char *fmt, ...) { XX(Logger::global(), 1); }
void debug(const char *fmt, ...) { XX(Logger::global(), 1); }

#undef XX

} // namespace mk
