# NAME
Logger &mdash; Log messages processor

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/common.hpp>

// The numbers [0-31] are reserved for verbosity levels.
#define MK_LOG_WARNING 0
#define MK_LOG_INFO 1
#define MK_LOG_DEBUG 2
#define MK_LOG_DEBUG2 3
#define MK_LOG_VERBOSITY_MASK 31

// Number above 31 have different semantics:
#define MK_LOG_EVENT 32          // Event occurred (encoded as JSON)

namespace mk {

class Logger : public NonCopyable, public NonMovable {
  public:
    static Var<Logger> make();

    void logv(uint32_t level, const char *fmt, va_list ap);
    void log(uint32_t level, const char *fmt, ...);
    void warn(const char *fmt, ...);
    void info(const char *fmt, ...);
    void debug(const char *fmt, ...);

    void set_verbosity(uint32_t v);
    void increase_verbosity();
    uint32_t get_verbosity();

    void on_log(Delegate<uint32_t, const char *> fn);
    void on_eof(Delegate<> fn);
    void on_event(Delegate<const char *> fn);
    void on_progress(Delegate<double, const char *> fn);

    void set_logfile(std::string path);

    void progress(double);
    void set_progress_offset(double offset);
    void set_progress_scale(double offset);

    static Var<Logger> global();

    ~Logger();
};

/* Functions using the default logger: */

void log(uint32_T level, const char *fmt, ...);
void warn(const char *fmt, ...);
void info(const char *fmt, ...);
void debug(const char *fmt, ...);

void set_verbosity(uint32_t v);
void increase_verbosity();
uint32_t get_verbosity();

void on_log(Delegate<uint32_t, const char *> fn);

void set_logfile(std::string path);

}
```

# DESCRIPTION

The `MK_LOG_XXX` macros allow to set verbosity levels and other
reserved values. Only the first four bits are used to represent levels
of verbosity, while other bits are used for other purposes.

The `Logger` class specifies how logs are processed. You can change the
function that receives logs. You can change the verbosity level. And
of course you can log messages as well.

The `make()` factory creates a logger wrapped by a shared pointer.

The `logv()`, `log()`, `warn()`, `info()`, and `debug()` methods allow to write
log messages. Specifically, `logv()` and `log()` take an explicit logging
level argument, while other functions provide it implicitly. Messages
are only written if the current verbosity level is not lower than the verbosity
level of the currently logged message. That is,

```C++
    Var<Logger> logger = Logger::make();
    logger->set_verbosity(MK_LOG_INFO);
    logger->info("This message will be printed");
    logger->debug("This one won't");
```

The `set_verbosity()`, `increase_verbosity()` and `get_verbosity()` methods
allow to manage the verbosity level of the logger. The default verbosity
level is `MK_LOG_WARNING` meaning that messages with verbosity greater than
that are not logged by default.

The `on_log` method allow to either set or reset the function called
for each logging message. Such function is a delegate (i.e. the delegate
body can safely change the delegate itself) and takes in input the
verbosity level of the message and the message itself. The default log
function prints on the standard error output the severity (unless the
severity is INFO, in which case nothing is printed) followed by the log
message. For example:

```
warning: A warning message
A info message
debug: A debug message
debug: A debug2 message
```

To disable the log callback, pass `nullptr` to it.

The `on_eof()` method allows to register a function to be called when the
logger is about to be destroyed. You can call this function multiple times
to register multiple callbacks, if you wish.

By default `MK_LOG_EVENT` messages are passed to the log callback. But you
can route them to the event callback by specifying it using `on_event()`. In
such case, those messages will be passed to the event callback only,
meaning that the log callback will not be called for them and they will
not be written on the logfile. This behavior is meant to transition between
when events where passed to the log callback and a future where they will
be either ignored or passed to the event callback.

The `on_progress` method allows to register a delegate to receive
progress notifications from measurement-kit tests. A progress notification
is a tuple composed of a double (between 0.0 and 1.0) and a string: the
double represents the overall percentage of completion whereas the string
represents the operation currently in progress.

The `set_logfile` method instructs the logger to write a copy of each log
message into the specified log file. Setting the logfile has no impact on
logs written using `on_log` and *viceversa*. By default no log file is
specified. It is legal (albeit useless) to have a logger not attached to
any log file and whose `on_log` callback is `nullptr`.

The `progress` method is used to emit progress notifications.

The `set_progress_offset` and `set_progress_scale` methods allow to define,
respectively, the offset to be added to progress notifications and the
scale value to multiply the progress notification for. By default the offset
is 0.0 and the scale is 1.0. These two methods are useful to normalize the
progress emitted by individual operations (which see _their_ progress as
a number between 0.0 and 1.0) in the context of a more general progress; e.g.,
the `MultiNdt` test runs two NDT tests, one using a single TCP stream and
the other using three TCP streams. Both NDT tests sees their progress as
between 0.0 and 1.0 but the parent `MultiNdt` class sets the scale equal to
0.5 and the offset of the second NDT test equal to 0.5, so to normalize
the progress emitted by the child NDT tests.

The `global()` factory returns the default logger.

The destructor calls the functions registered using `on_eof`.

This module also includes syntactic sugar functions named like `Logger`
methods that call the namesake method of the default logger. That is:

```C++
  mk::debug("Foobar");  // == mk::Logger::global()->debug("Foobar");
```

Internally, the logger implementation MUST be implemented to be thread
safe, i.e. it MUST be safe to invoke concurrently the logger from multiple
threads, *as far as* the consistency of the logger internals is
concerned. Note that this *does not mean at all* that the logger delegate
can safely access resources owned by another thread. In such case, it is
the programmer's responsibility than any relevant shared resources are
locked before they are used, i.e., you MUST use this pattern:

```C++
    mk::on_log([=](uint32_t level, const char *message) {
        resource.acquire_lock();
        // Then you can process the message
    });
```

Also note that the implementation MAY use a shared internal buffer meaning that
log messages SHOULD either immediately consumed or cached if delayed consumption
is planned. Failure to do so would possibly lead to the latest produced log
message printed more than once. Instead use this pattern:

```C++
    mk::on_log([=](uint32_t level, const char *message) {
        resource.acquire_lock();
        std::string copy{message};
        resource.sched_deferred_consumption(level, copy);
    });
```

Also, note that, being thread safe, the logger MUST lock its internals before
emitting a log message. Thus, you MUST NOT call the logger from a logger callback
because this MAY result in deadlock or internal buffer corruption, depending on
the mutex implementation. Ideally, the log message should be printed on some
file, or you should save it as described above and then schedule a delayed call to
properly process the message, if processing it is going to be slow.

You MUST NOT set up the logger from multiple thread contexts, because methods
that set callbacks, change the logger behavior, etc., are not thread safe.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

using namespace mk;

int main() {
    Var<Logger> logger = Logger::make();

    logger->set_verbosity(MK_LOG_DEBUG);
    logger->on_log([](uint32_t level, const char *log_line) {
        printf("<%d> %s\n", level, log_line);
    });

    logger->debug("Format string: %s", "but also arguments");
    logger->info("Just like printf");
    logger->warn("Use this for important messages");
}
```

# BUGS

If the logfile could not be open, or written, the error is silently
discarded (i.e. no exceptions thrown, no return values).

# HISTORY

The `Logger` class appeared in MeasurementKit 0.1.0.
