// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <assert.h>
#include <cstdint>
#include <fstream>
#include <list>
#include <measurement_kit/common/aaa_base.h>
#include <measurement_kit/common/callback.hpp>
#include "src/libmeasurement_kit/common/delegate.hpp"
#include "src/libmeasurement_kit/common/locked.hpp"
#include <measurement_kit/common/json.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>

namespace mk {

class DefaultLogger : public Logger, public NonCopyable, public NonMovable {
  public:
    DefaultLogger() {
        consumer_ = [](uint32_t level, const char *s) {
            std::string message;
            if ((level & MK_LOG_EVENT) != 0) {
                Error err = json_process(
                    s, [&](auto j) { message = j.dump(4); });
                if (err) {
                    fprintf(
                        stderr, "warning: logger cannot parse json message\n");
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

    void logv(uint32_t level, const char *fmt, va_list ap) override {
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
                static_assert(
                    sizeof(buffer_) >= sizeof(eb), "buffer_ too short");
                memcpy(buffer_, eb, sizeof(eb));
                // FALLTHROUGH
            }
            level = MK_LOG_WARNING;

        } else if ((unsigned int)res >= sizeof(buffer_)) {
            static_assert(sizeof(buffer_) >= 4, "buffer_ too short");
            buffer_[sizeof(buffer_) - 1] = '\0';
            buffer_[sizeof(buffer_) - 2] = '.';
            buffer_[sizeof(buffer_) - 3] = '.';
            buffer_[sizeof(buffer_) - 4] = '.';

        } else {
            /* NOTHING */;
        }

        // Since v0.4 we dispatch the MK_LOG_EVENT event to the proper handler
        // if set, otherwise we fallthrough passing it to consumer_.
        if (event_handler_ and (level & MK_LOG_EVENT) != 0) {
            try {
                event_handler_(buffer_);
            } catch (const std::exception &) {
                /* Suppress */;
            }
            return;
        }

        if (consumer_) {
            try {
                consumer_(level, buffer_);
            } catch (const std::exception &) {
                /* Suppress */;
            }
        }

        if (ofile_) {
            // FIX: use `std::endl` rather than `\n` to make sure we flush
            // after each line. Fixes TheTorProject/ooniprobe-ios#80.
            *ofile_ << buffer_ << std::endl;
            // TODO: suppose here write fails... what do we want to do?
        }
    }

#define XX(_logger_, _level_)                                                  \
    do {                                                                       \
        uint32_t real_level = (_level_)&MK_LOG_VERBOSITY_MASK;                 \
        if (real_level <= _logger_->get_verbosity()) {                         \
            va_list ap;                                                        \
            va_start(ap, fmt);                                                 \
            _logger_->logv(_level_, fmt, ap);                                  \
            va_end(ap);                                                        \
        }                                                                      \
    } while (0)

    void log(uint32_t level, const char *fmt, ...) override { XX(this, level); }

    void err(const char *fmt, ...) override { XX(this, MK_LOG_ERR); }

    void warn(const char *fmt, ...) override { XX(this, MK_LOG_WARNING); }

    void info(const char *fmt, ...) override { XX(this, MK_LOG_INFO); }

    void debug(const char *fmt, ...) override { XX(this, MK_LOG_DEBUG); }

    void debug2(const char *fmt, ...) override { XX(this, MK_LOG_DEBUG2); }

    void set_verbosity(uint32_t v) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        verbosity_ = (v & MK_LOG_VERBOSITY_MASK);
    }

    void increase_verbosity() override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (verbosity_ < MK_LOG_VERBOSITY_MASK) {
            ++verbosity_;
        }
    }

    uint32_t get_verbosity() override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        return verbosity_;
    }

    void on_log(Callback<uint32_t, const char *> &&fn) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        consumer_ = std::move(fn);
    }

    void on_eof(Callback<> &&f) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        eof_handlers_.push_back(std::move(f));
    }

    void on_event(Callback<const char *> &&f) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        event_handler_ = std::move(f);
    }

    void on_event_ex(const std::string &event,
            Callback<nlohmann::json &&> &&cb) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        handlers_[event] = std::move(cb);
    }

    void on_progress(Callback<double, const char *> &&fn) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        progress_handler_ = fn;
    }

    void set_logfile(std::string path) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        ofile_.reset(new std::ofstream(path));
        // TODO: what to do if we cannot open the logfile? return error?
    }

    void progress(double prog, const char *s) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (progress_handler_) {
            prog = prog * progress_scale_ + progress_offset_;
            try {
                progress_handler_(prog, s);
            } catch (const std::exception &) {
                /* Suppress */;
            }
        }
        assert(!!s);
        // Note that the mutex is recursive
        // TODO(bassosimone): improve the API to allow emitting more context
        emit_event_ex("status.progress", {
            {"percentage", prog},
            {"message", s},
        });
    }

    void emit_event_ex(
            const std::string &key, nlohmann::json &&value) override {
        if (!value.is_object()) {
            warn("wrong value for key: %s", key.c_str());
            assert(false);
        }
        if (handlers_.count(key) <= 0) {
            return;
        }
        nlohmann::json event{
            {"key", key},
            {"value", std::move(value)}
        };
        std::unique_lock<std::recursive_mutex> _{mutex_};
        // TODO(bassosimone): other logging functions filter all the
        // exceptions. We cannot change this behavior until that is part
        // of our public API. But here we deliberately choose not to do
        // any exception handling. The callee must behave.
        handlers_.at(key)(std::move(event));
    }

    void progress_relative(double prog, const char *s) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (progress_handler_) {
            progress_relative_ += prog * progress_scale_;
            try {
                progress_handler_(progress_offset_ + progress_relative_, s);
            } catch (const std::exception &) {
                /* Suppress */;
            }
        }
    }

    void set_progress_offset(double offset) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        progress_offset_ = offset;
        progress_relative_ = 0.0;
    }

    void set_progress_scale(double scale) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        progress_scale_ = scale;
    }

    ~DefaultLogger() override {
        for (auto f : eof_handlers_) {
            try {
                f();
            } catch (const std::exception &) {
                /* Suppress */;
            }
        }
    }

  private:
    Delegate<uint32_t, const char *> consumer_;
    uint32_t verbosity_ = MK_LOG_WARNING;
    char buffer_[32768];
    std::recursive_mutex mutex_;
    SharedPtr<std::ofstream> ofile_;
    std::list<Delegate<>> eof_handlers_;
    Delegate<const char *> event_handler_;
    std::map<std::string, Delegate<nlohmann::json &&>> handlers_;
    Delegate<double, const char *> progress_handler_;
    double progress_offset_ = 0.0;
    double progress_scale_ = 1.0;
    double progress_relative_ = 0.0;
};

/*static*/ SharedPtr<Logger> Logger::make() {
    return SharedPtr<Logger>{std::make_shared<DefaultLogger>()};
}

/*static*/ SharedPtr<Logger> Logger::global() {
    return locked_global([]() {
        static SharedPtr<Logger> singleton = Logger::make();
        return singleton;
    });
}

Logger::~Logger() {}

void log(uint32_t level, const char *fmt, ...) { XX(Logger::global(), level); }

void err(const char *fmt, ...) { XX(Logger::global(), MK_LOG_ERR); }

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
