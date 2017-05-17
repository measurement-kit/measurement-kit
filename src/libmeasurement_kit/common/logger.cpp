// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <stdio.h>

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>

namespace mk {

/*static*/ Var<Logger> Logger::make() { return Var<Logger>(new Logger); }

Logger::Logger() {
    consumer_ = [](uint32_t level, const char *s) {
        std::string message;
        if ((level & MK_LOG_EVENT) != 0) {
            try {
                message = nlohmann::json::parse(s).dump(4);
                s = message.c_str();
            } catch (std::exception &) {
                fprintf(stderr, "warning: logger cannot parse json message\n");
                return;
            }
            /* FALLTHROUGH */
        }
        uint32_t verbosity = (level & MK_LOG_VERBOSITY_MASK);
        if (verbosity <= MK_LOG_WARNING) {
            fprintf(stderr, "[!] %s\n", s);
        } else if (verbosity == MK_LOG_INFO) {
            fprintf(stderr, "%s\n", s);
        } else {
            fprintf(stderr, "[D] %s\n", s);
        }
    };
}

void Logger::logv(uint32_t level, const char *fmt, va_list ap) {
    if (!consumer_ and !ofile_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int res = vsnprintf(buffer_, sizeof(buffer_), fmt, ap);

    // Once we know that res is non-negative we make it unsigned,
    // which allows the compiler to promote the smaller of res and
    // sizeof (buffer) to the correct size if needed.
    if (res < 0) {
        res = snprintf(buffer_, sizeof(buffer_),
                       "logger: cannot format message with level %d "
                       "and format string '%s' (vsnprintf() returned: %d)",
                       level, fmt, res);
        if (res < 0 || (unsigned int)res >= sizeof(buffer_)) {
            static const char eb[] = "logger: cannot format message";
            static_assert(sizeof (buffer_) >= sizeof (eb), "buffer_ too short");
            memcpy(buffer_, eb, sizeof (eb));
            // FALLTHROUGH
        }
        level = MK_LOG_WARNING;

    } else if ((unsigned int)res >= sizeof(buffer_)) {
        static_assert(sizeof(buffer_) >= 4, "buffer_ too short");
        buffer_[sizeof (buffer_) - 1] = '\0';
        buffer_[sizeof (buffer_) - 2] = '.';
        buffer_[sizeof (buffer_) - 3] = '.';
        buffer_[sizeof (buffer_) - 4] = '.';

    } else {
        /* NOTHING */ ;
    }

    // Since v0.4 we dispatch the MK_LOG_EVENT event to the proper handler
    // if set, otherwise we fallthrough passing it to consumer_.
    if (event_handler_ and (level & MK_LOG_EVENT) != 0) {
        try {
            event_handler_(buffer_);
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
        return;
    }

    if (consumer_) {
        try {
            consumer_(level, buffer_);
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
    }

    if (ofile_) {
        *ofile_ << buffer_ << "\n";
        // TODO: suppose here write fails... what do we want to do?
    }
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

void Logger::on_eof(Delegate<> f) {
    eof_handlers_.push_back(f);
}

void Logger::on_event(Delegate<const char *> f) { event_handler_ = f; }

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

void Logger::on_progress(Delegate<double, const char *> fn) {
    progress_handler_ = fn;
}

void Logger::set_logfile(std::string path) {
    ofile_.reset(new std::ofstream(path));
    // TODO: what to do if we cannot open the logfile? return error?
}

void Logger::progress(double prog, const char *s) {
    if (progress_handler_) {
        prog = prog * progress_scale_ + progress_offset_;
        try {
            progress_handler_(prog, s);
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
    }
}

void Logger::set_progress_offset(double offset) {
    progress_offset_ = offset;
}

void Logger::set_progress_scale(double scale) {
    progress_scale_ = scale;
}

Logger::~Logger() {
    for (auto f : eof_handlers_) {
        try {
            f();
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
    }
}

} // namespace mk
