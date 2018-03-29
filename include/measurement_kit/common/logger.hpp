// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#include <cstdint>
#include <measurement_kit/common/aaa_base.h>
#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/json.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <stdarg.h>

// The numbers [0-31] are reserved for verbosity levels.

/// `MK_LOG_QUIET` indicates that no log should be emitted.
#define MK_LOG_QUIET 0

/// `MK_LOG_ERR` indicates the `ERR` log severity level.
#define MK_LOG_ERR 1

/// `MK_LOG_WARNING` indicates the `WARNING` log severity level.
#define MK_LOG_WARNING 2

/// `MK_LOG_INFO` indicates the `INFO` log severity level.
#define MK_LOG_INFO 3

/// `MK_LOG_DEBUG` indicates the `DEBUG` log severity level.
#define MK_LOG_DEBUG 4

/// `MK_LOG_DEBUG2` indicates the `DEBUG2` log severity level.
#define MK_LOG_DEBUG2 5

/// \brief `MK_LOG_VERBOSITY_MASK` is a bitmask indicating which bits
/// are being used to specify the severity level. Bits above such mask
/// have another semantic.
#define MK_LOG_VERBOSITY_MASK 31

// The number above 31 have different semantics:

/// \brief `MK_LOG_EVENT` indicates an event. It is a bit outside of the
/// verbosity mask. This is used to indicate that the current log message
/// is not plaintext but rather a serialized JSON representing an event.
#define MK_LOG_EVENT 32

// Note: the attribute we use below is GCC and Clang specific (and Clang
// identifies itself as GCC), so make sure other compilers are not going to
// see the attribute definition, which will break the build.
#ifndef __GNUC__
#define __attribute__(x_) /* Nothing */
#endif

namespace mk {

/// \brief `Logger` specifies how logs are processed. It is an abstract class
/// usually accessed through SharedPtr, because there can be different
/// implementations of the logger.
///
/// You can change the verbosity level of the logger, using set_verbosity()
/// and/or increase_verbosity(). The default verbosity is MK_LOG_WARNING
/// meaning that messages with severity lower than warning will not be printed
/// by the logger.
///
/// You can set the function where to log, using on_log(). You can also
/// decide to write the logger output to a file, that you can specify
/// using the set_logfile() method.
///
/// \bug In the default implementation of Logger, if the log file could
/// not be open or written, such error is silently ignored.
///
/// \note All methods of the default logger implementation are protected
/// against access from multiple threads by a recursive mutex.
///
/// \since v0.1.0.
class Logger {
  public:
    /// `make()` creates an instance of the default logger.
    static SharedPtr<Logger> make();

    /// `global()` returns the global instance of the default logger.
    static SharedPtr<Logger> global();

    /// \brief `logv()` writes a log message. \param mask is a mask of the
    /// verbosity level (e.g. MK_LOG_DEBUG) and possibly other
    /// specifiers (e.g. MK_LOG_EVENT). \param fmt is the format
    /// string. \param ap is the argument pointer.
    ///
    /// In the default logger, the `logv` function behaves as follows:
    ///
    /// The log message will not be written if its verbosity is
    /// lower than the currently configured verbosity.
    ///
    /// If you specify MK_LOG_EVENT and you don't override the default
    /// log handler with on_log() and you don't set a specific
    /// MK_LOG_EVENT handler with on_event(), the code will check whether
    /// the log message is a valid JSON and print an error otherwise.
    ///
    /// If the verbosity level is such that a log message must be
    /// printed, the default logger will use vsnprintf() for formatting
    /// the message. If vsnprintf() fails because formatting is not
    /// possible, the logger will tell you that. If in any event the
    /// internal log buffer is too short, the resulting log message
    /// will reflect that by writing `"..."` at the end.
    ///
    /// If MK_LOG_EVENT is specified and you set an handler using
    /// on_event(), the message will _only_ be passed to such handler,
    /// otherwise it will be passed to the handler specified using
    /// on_log(), and otherwise to the default log handler.
    ///
    /// Processing stops here if you specified MK_LOG_EVENT and you
    /// specified also an event handler using on_event(). Otherwise:
    ///
    /// The message will be passed to the log handler (either the default
    /// one or one that you set with on_log()). Note that you can disable the
    /// log handler with:
    ///
    /// ```C++
    /// logger->on_log(nullptr);
    /// ```
    ///
    /// If you set a logfile with set_logfile(), the message will _also_
    /// be written into the logfile.
    virtual void logv(uint32_t mask, const char *fmt, va_list ap)
        __attribute__((format(printf, 3, 0))) = 0;

    /// `log()` calls logv().
    virtual void log(uint32_t, const char *, ...)
        __attribute__((format(printf, 3, 4))) = 0;

    /// `err()` calls logv() with MK_LOG_ERR.
    virtual void err(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    /// `warn()` calls logv() with MK_LOG_WARNING.
    virtual void warn(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    /// `info()` calls logv() with MK_LOG_INFO.
    virtual void info(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    /// `debug()` calls logv() with MK_LOG_DEBUG.
    virtual void debug(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    /// `debug2()` calls logv() with MK_LOG_DEBUG2.
    virtual void debug2(const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) = 0;

    /// `set_verbosity()` allows to set the logger verbosity.
    virtual void set_verbosity(uint32_t v) = 0;

    /// \brief `increase_verbosity()` increases the logger verbosity. That is,
    /// if the current verbosity is MK_LOG_INFO, increasing the verbosity
    /// will set as new verbosity MK_LOG_DEBUG.
    virtual void increase_verbosity() = 0;

    /// `get_verbosity()` gets the current verbosity.
    virtual uint32_t get_verbosity() = 0;

    /// `on_log` allows to set the log handler.
    virtual void on_log(Callback<uint32_t, const char *> &&fn) = 0;

    // TODO(bassosimone): when this header will become private, we can then
    // remove on_eof() and on_event() because they'll become unused. We will
    // also be able to remove the progress handler.

    /// \brief `on_eof()` allows to set the EOF handler. You can set more
    /// than one handler. All the set handlers will be called when the
    /// logger is destroyed. This is used e.g. in Android to free resources.
    virtual void on_eof(Callback<> &&fn) = 0;

    /// `on_event()` allows to set the MK_LOG_EVENT handler.
    virtual void on_event(Callback<const char *> &&fn) = 0;

    /// `on_event_ex()` registers a handler for the specified event.
    virtual void on_event_ex(const std::string &event,
                             Callback<nlohmann::json &&> &&cb) = 0;

    /// \brief `on_progress()` allows to set the progress handler. Progress
    /// is emitted when the test proceeeds.
    virtual void on_progress(Callback<double, const char *> &&fn) = 0;

    /// `set_logfile()` sets the file where to write logs.
    virtual void set_logfile(std::string fpath) = 0;

    /// `emit_event_ex()` emits an event as a JSON.
    virtual void emit_event_ex(const std::string &key,
                               nlohmann::json &&value) = 0;

    /// \brief `progress()` emits a progress event. \param percent is the
    /// percentage of completion of the current test. \param message is the
    /// string describing what the test is currently doing.
    ///
    /// For example:
    ///
    /// ```C++
    /// logger->progress(0.1 /* 10% */, "Contacting the bouncer");
    /// ```
    virtual void progress(double percent, const char *message) = 0;

    /// \brief `progres_relative()` emits the _relative_ progress. This is
    /// just like progress() except that \p delta is not an absolute
    /// percentage indicator, rather a percentage to be added to the current
    /// level of completion of the test. Of course, this means that the
    /// logger will keep track of the current progress for you.
    ///
    /// \note The presence of both progress() and progress_relative() is
    /// probably confusing and we should deprecate progress().
    virtual void progress_relative(double delta, const char *message) = 0;

    /// \brief `set_progress_offset()` sets the current offset. Further
    /// invocations of progress() or progress_relative() will sum the
    /// specified offset to the one passed to them as argument.
    virtual void set_progress_offset(double offset) = 0;

    /// \brief `set_progress_scale()` sets the progress scale. Further
    /// invocations of progress() or progress_offset() will have their
    /// argument multiplied by the selected scale.
    virtual void set_progress_scale(double scale) = 0;

    /// `~Logger()` destroys allocated resources.
    virtual ~Logger();
};

/// `log()` call the Logger::log() method of the global logger.
void log(uint32_t, const char *, ...) __attribute__((format(printf, 2, 3)));

/// `err()` call the Logger::err() method of the global logger.
void err(const char *, ...) __attribute__((format(printf, 1, 2)));

/// `warn()` call the Logger::warn() method of the global logger.
void warn(const char *, ...) __attribute__((format(printf, 1, 2)));

/// `info()` call the Logger::info() method of the global logger.
void info(const char *, ...) __attribute__((format(printf, 1, 2)));

/// `debug()` call the Logger::debug() method of the global logger.
void debug(const char *, ...) __attribute__((format(printf, 1, 2)));

/// `debug2()` call the Logger::debug2() method of the global logger.
void debug2(const char *, ...) __attribute__((format(printf, 1, 2)));

/// \brief `set_verbosity()` call the Logger::set_verbosity() method of the
/// global logger.
void set_verbosity(uint32_t v);

/// \brief `increase_verbosity()` call the Logger::increase_verbosity()
/// method of the global logger.
void increase_verbosity();

/// \brief `get_verbosity()` call the Logger::get_verbosity()
/// method of the global logger.
uint32_t get_verbosity();

/// \brief `on_log()` call the Logger::on_log()
/// method of the global logger.
void on_log(Callback<uint32_t, const char *> &&fn);

/// \brief `set_logfile()` call the Logger::set_logfile()
/// method of the global logger.
void set_logfile(std::string path);

} // namespace mk

#ifndef __GNUC__
#undef __attribute__
#endif

#endif
