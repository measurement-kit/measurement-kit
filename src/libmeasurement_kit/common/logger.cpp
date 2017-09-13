// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/detail/json.hpp>
#include <measurement_kit/common/detail/locked.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/ext/json.hpp>
#include <stdio.h>

namespace mk {

/*static*/ SharedPtr<Logger> Logger::make() {
    return std::make_shared<Logger>();
}

Logger::Logger() {
    consumer_ = [](uint32_t level, const char *s) {
        std::string message;
        if ((level & MK_LOG_EVENT) != 0) {
            Error err = json_parse_and_process(
                  s, [&](auto j) { message = j.dump(4); });
            if (err) {
                fprintf(stderr, "warning: logger cannot parse json message\n");
                return;
            }
            s = message.c_str();
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
    std::unique_lock<std::recursive_mutex> _{mutex_};

    if (!consumer_ and !ofile_) {
        return;
    }

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

void Logger::debug2(const char *fmt, ...) { XX(this, MK_LOG_DEBUG2); }

void Logger::set_verbosity(uint32_t v) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    verbosity_ = (v & MK_LOG_VERBOSITY_MASK);
}

void Logger::increase_verbosity() {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    if (verbosity_ < MK_LOG_VERBOSITY_MASK) {
        ++verbosity_;
    }
}

uint32_t Logger::get_verbosity() {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    return verbosity_;
}

void Logger::on_log(Callback<uint32_t, const char *> &&fn) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    consumer_ = std::move(fn);
}

void Logger::on_eof(Callback<> &&f) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    eof_handlers_.push_back(std::move(f));
}

void Logger::on_event(Callback<const char *> &&f) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    event_handler_ = std::move(f);
}

void Logger::on_progress(Callback<double, const char *> &&fn) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    progress_handler_ = fn;
}

void Logger::set_logfile(std::string path) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    ofile_.reset(new std::ofstream(path));
    // TODO: what to do if we cannot open the logfile? return error?
}

void Logger::progress(double prog, const char *s) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    if (progress_handler_) {
        prog = prog * progress_scale_ + progress_offset_;
        try {
            progress_handler_(prog, s);
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
    }
}

void Logger::progress_relative(double prog, const char *s) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    if (progress_handler_) {
        progress_relative_ += prog * progress_scale_;
        try {
            progress_handler_(progress_offset_ + progress_relative_, s);
        } catch (const std::exception &) {
            /* Suppress */ ;
        }
    }
}

void Logger::set_progress_offset(double offset) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    progress_offset_ = offset;
    progress_relative_ = 0.0;
}

void Logger::set_progress_scale(double scale) {
    std::unique_lock<std::recursive_mutex> _{mutex_};
    progress_scale_ = scale;
}

/*static*/ SharedPtr<Logger> Logger::global() {
    return locked_global([]() {
        static SharedPtr<Logger> singleton = Logger::make();
        return singleton;
    });
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

void log(uint32_t level, const char *fmt, ...) { XX(Logger::global(), level); }

void warn(const char *fmt, ...) { XX(Logger::global(), MK_LOG_WARNING); }

void info(const char *fmt, ...) { XX(Logger::global(), MK_LOG_INFO); }

void debug(const char *fmt, ...) { XX(Logger::global(), MK_LOG_DEBUG); }

void debug2(const char *fmt, ...) { XX(Logger::global(), MK_LOG_DEBUG2); }

void set_verbosity(uint32_t v) { Logger::global()->set_verbosity(v); }

void increase_verbosity() { Logger::global()->increase_verbosity(); }

uint32_t get_verbosity() { return Logger::global()->get_verbosity(); }

void on_log(Callback<uint32_t, const char *> &&fn) {
    Logger::global()->on_log(std::move(fn));
}

void set_logfile(std::string path) { Logger::global()->set_logfile(path); }

#undef XX

} // namespace mk
