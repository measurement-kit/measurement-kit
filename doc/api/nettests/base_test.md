# NAME
BaseTest &mdash; Base network test class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

class BaseTest {
  public:
    BaseTest &on_logger_eof(Delegate<>);
    BaseTest &on_log(Delegate<uint32_t, const char *>);
    BaseTest &on_event(Delegate<const char *>);
    BaseTest &on_progress(Delegate<double, const char *>);
    BaseTest &set_verbosity(uint32_t);
    BaseTest &increase_verbosity();
    BaseTest &set_input_filepath(std::string);
    BaseTest &set_output_filepath(std::string);
    template <typename T> BaseTest &set_options(std::string, T);

    BaseTest &on_entry(Delegate<std::string>);
    BaseTest &on_begin(Delegate<>);
    BaseTest &on_end(Delegate<> cb);

    void run();
    void start(Callback<>);

    Var<Runnable> runnable;
};

```

# STABILITY

2 - Stable

# DESCRIPTION

`BaseTest` is the abstract base class of all tests. Tests should
subclass this class. `BaseTest` defines the abstract domain specific
language used to run tests.

The `on_logger_eof` method sets the delegate called when the logger
bound to the `BaseTest` test will be destroyed.

The `on_log` method sets the delegate called whenever the test
would emit a log message. The first argument would be the severity
and the second the log line; see `Logger` for more info.

The `on_event` method sets the delegate called whenever the test
produces an intermediate result consumable by the caller; e.g., the
`NDT` test would produce `download-speed` events every 0.5 s. In
general, events emitted through this method SHOULD be like:

```JSON
{
  "type": "type_name"
}
```

That is, they SHOULD be JSON objects containing the `type` key bound
to a string value indicating the event type. All other fields would
be event dependent.

The `on_progress` method allows to set the delegate called to indicate
the caller that the test is making progress. The first argument is a
number (comprised between zero and one) indicating the percentage of
completion of the test. The second argument is a string describing to
the user the current operation being performed by the test.

The `set_verbosity` and the `increase_verbosity` methods allow to,
respectively, set an increase the verbosity of the logger bound to
this test.

The `set_input_filepath` and `set_output_filepath` methods allow to
set, respectively, the input file path (i.e. the path of the file
containing the inputs that the test should consume) and the output
file path (i.e. the file where the test should write the results
of the test). By default there is no input file path and the output
file path is the empty string. In that case, the test will generate
an output file path appropriate for the test itself. If you want
to disable writing output to disk, you should pass the test the
`no_file_report` setting instead, by setting its value to false.

The `set_options` method allows to specify test options. Note that
all options values SHOULD be passed as string (future versions of
measurement-kit MAY) disable the possibility of passing test values
as arbitrary scalar values, and the values are currently converted
to strings internally anyway.

The `on_entry` method allows to specify the delegate called when
a test entry is about to be written to disk. The first argument
receives the entry object serialized as JSON. Note that the entry
format MUST be compatible with [OONI specification](https://github.com/TheTorProject/ooni-spec/tree/master/data-formats).

The `on_begin` and `on_end` methods allow to specify delegates called
respectively when the test is about to begin and when the test is about
to end. They may be useful to start and stop filtering for tests
related events, for example. You MAY specify multiple `on_end` callbacks
if you wish; they will be executed sequentially at end of test.

The `run` method runs the test synchronously. It will not return until
the test has terminated. Note that measurement-kit MAY run the test
in the current thread or in a background thread.

The `start` method starts the test on a background thread. The callback
passed as argument will be called when the test is terminated.

The `runnable` public attribute contains the `Runnable` instance that
will be scheduled when you call either `run` or `start`. Note that the
`runnable` attribute is a smart pointer. The pointee is guaranteed to
be a valid `runnable` object since the `BaseTest` is created until the
`run` or `start` methods are called; afterwards, the pointee MAY be
`nullptr`. Thus attempting to call `start` or `run` again MAY result
in an exception being thrown.

The domain-specific-language initialization style, combined with either `run`
or start`, allows to express the intention of running a specific test
in a very compact and declarative way:

```C++
    FooTest()
        .set_options("foo", false)
        .set_options("/tmp/file.njson")
        .run();
```

Because `start` executes the test in a background thread and `run` MAY do
the same, *make sure you lock shared resources* inside the callback; i.e.:

```C++
    FooTest()
        .set_options("foo", false)
        .set_options("/tmp/file.njson")
        .run([=]() {
            shared_resource.lock();
            // Do something with shared resource
        });
```

# HISTORY

The `NetTest` class appeared in MeasurementKit 0.1.0 in the `common`
namespace. It was moved in the `nettests` namespace, significantly
refactored, and renamed as `BaseTest` in 0.4.0.
