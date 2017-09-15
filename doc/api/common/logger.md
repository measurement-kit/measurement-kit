# NAME

`measurement_kit/common/logger.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

#define MK_LOG_WARNING 0

#define MK_LOG_INFO 1

#define MK_LOG_DEBUG 2

#define MK_LOG_DEBUG2 3

#define MK_LOG_VERBOSITY_MASK 31

#define MK_LOG_EVENT 32

namespace mk {

class Logger {
  public:
    static SharedPtr<Logger> make();

    static SharedPtr<Logger> global();

    virtual void logv(uint32_t mask, const char *fmt, va_list ap)
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

    virtual void progress(double percent, const char *message) = 0;

    virtual void progress_relative(double delta, const char *message) = 0;

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
```

# DESCRIPTION

`MK_LOG_WARNING` indicates the `WARNING` log severity level.

`MK_LOG_INFO` indicates the `INFO` log severity level.

`MK_LOG_DEBUG` indicates the `DEBUG` log severity level.

`MK_LOG_DEBUG2` indicates the `DEBUG2` log severity level.

`MK_LOG_VERBOSITY_MASK` is a bitmask indicating which bits are being used to specify the severity level. Bits above such mask have another semantic.

`MK_LOG_EVENT` indicates an event. It is a bit outside of the verbosity mask. This is used to indicate that the current log message is not plaintext but rather a serialized JSON representing an event.

`Logger` specifies how logs are processed. It is an abstract class usually accessed through SharedPtr, because there can be different implementations of the logger. 

You can change the verbosity level of the logger, using set_verbosity() and/or increase_verbosity(). The default verbosity is MK_LOG_WARNING meaning that messages with severity lower than warning will not be printed by the logger. 

You can set the function where to log, using on_log(). You can also decide to write the logger output to a file, that you can specify using the set_logfile() method. 

_BUG_: In the default implementation of Logger, if the log file could not be open or written, such error is silently ignored. 

_Note_: All methods of the default logger implementation are protected against access from multiple threads by a recursive mutex. 

Appeared in measurement-kit v0.1.0.

`make()` creates an instance of the default logger.

`global()` returns the global instance of the default logger.

`logv()` writes a log message. Parameter mask is a mask of the verbosity level (e.g. MK_LOG_DEBUG) and possibly other specifiers (e.g. MK_LOG_EVENT). Parameter fmt is the format string. Parameter ap is the argument pointer. 

In the default logger, the `logv` function behaves as follows: 

The log message will not be written if its verbosity is lower than the currently configured verbosity. 

If you specify MK_LOG_EVENT and you don't override the default log handler with on_log() and you don't set a specific MK_LOG_EVENT handler with on_event(), the code will check whether the log message is a valid JSON and print an error otherwise. 

If the verbosity level is such that a log message must be printed, the default logger will use vsnprintf() for formatting the message. If vsnprintf() fails because formatting is not possible, the logger will tell you that. If in any event the internal log buffer is too short, the resulting log message will reflect that by writing `"..."` at the end. 

If MK_LOG_EVENT is specified and you set an handler using on_event(), the message will _only_ be passed to such handler, otherwise it will be passed to the handler specified using on_log(), and otherwise to the default log handler. 

Processing stops here if you specified MK_LOG_EVENT and you specified also an event handler using on_event(). Otherwise: 

The message will be passed to the log handler (either the default one or one that you set with on_log()). Note that you can disable the log handler with: 

```C++ logger->on_log(nullptr); ``` 

If you set a logfile with set_logfile(), the message will _also_ be written into the logfile.

`log()` calls logv().

`warn()` calls logv() with MK_LOG_WARNING.

`info()` calls logv() with MK_LOG_INFO.

`debug()` calls logv() with MK_LOG_DEBUG.

`debug2()` calls logv() with MK_LOG_DEBUG2.

`set_verbosity()` allows to set the logger verbosity.

`increase_verbosity()` increases the logger verbosity. That is, if the current verbosity is MK_LOG_INFO, increasing the verbosity will set as new verbosity MK_LOG_DEBUG.

`get_verbosity()` gets the current verbosity.

`on_log` allows to set the log handler.

`on_eof()` allows to set the EOF handler. You can set more than one handler. All the set handlers will be called when the logger is destroyed. This is used e.g. in Android to free resources.

`on_event()` allows to set the MK_LOG_EVENT handler.

`on_progress()` allows to set the progress handler. Progress is emitted when the test proceeeds.

`set_logfile()` sets the file where to write logs.

`progress()` emits a progress event. Parameter percent is the percentage of completion of the current test. Parameter message is the string describing what the test is currently doing. 

For example: 

```C++ logger->progress(0.1 /* 10% */, "Contacting the bouncer"); ```

`progres_relative()` emits the _relative_ progress. This is just like progress() except that delta is not an absolute percentage indicator, rather a percentage to be added to the current level of completion of the test. Of course, this means that the logger will keep track of the current progress for you. 

_Note_: The presence of both progress() and progress_relative() is probably confusing and we should deprecate progress().

`set_progress_offset()` sets the current offset. Further invocations of progress() or progress_relative() will sum the specified offset to the one passed to them as argument.

`set_progress_scale()` sets the progress scale. Further invocations of progress() or progress_offset() will have their argument multiplied by the selected scale.

`~Logger()` destroys allocated resources.

`log()` call the Logger::log() method of the global logger.

`warn()` call the Logger::warn() method of the global logger.

`info()` call the Logger::info() method of the global logger.

`debug()` call the Logger::debug() method of the global logger.

`debug2()` call the Logger::debug2() method of the global logger.

`set_verbosity()` call the Logger::set_verbosity() method of the global logger.

`increase_verbosity()` call the Logger::increase_verbosity() method of the global logger.

`get_verbosity()` call the Logger::get_verbosity() method of the global logger.

`on_log()` call the Logger::on_log() method of the global logger.

`set_logfile()` call the Logger::set_logfile() method of the global logger.

