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
    BaseTest &add_input(std::string);
    BaseTest &add_input_filepath(std::string);
    BaseTest &set_input_filepath(std::string);
    BaseTest &set_output_filepath(std::string);
    template <typename T> BaseTest &set_options(std::string, T);

    BaseTest &on_entry(Delegate<std::string>);
    BaseTest &on_begin(Delegate<>);
    BaseTest &on_end(Delegate<> cb);
    BaseTest &on_destroy(Delegate<> cb);

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

The `add_input` method adds one specific input to the list of inputs to
be processed by this test. If the test does not take input, the input added
using this method will be ignored. Otherwise, it will be processed at the
beginning of the test, before input read from files (see below) is processed.
You can call this method multiple times to manually add multiple input.

The `add_input_filepath` adds one file path to the input paths list, which
by default is empty. Tests that require input will fail if no input file
path is specified and no input was provided using `add_input`,
while tests that do not require input will ignore input
if it is provided by the caller. If an input file path cannot be openned
or read, measurement-kit will emit a warning message, but the test will not
fail. If an input file path contains the string "${probe_cc}" this is
replaced with the two letter country code of the network the user is running
the test from in lower case (that is, if you are in Italy &mdash; country
code `IT` &mdash; the string "${probe_cc}" is replaced with `it`).

The `set_input_filepath` method clears previously set input file paths
and then sets the path provided as argument as the unique input file path.

The `set_output_filepath` method allows to set the output file path (i.e.
the file where the test should write the results of the test). If you don't
specify the output file path, the test will generate an output file path
appropriate for the test itself. If you want to disable writing output to
disk, you should pass the test the `no_file_report` setting instead, by
setting its value to the string "0".

The `set_options` method allows to specify test options. Note that
all options values SHOULD be passed as string (future versions of
measurement-kit MAY disable the possibility of passing test values
as arbitrary scalar values, and the values are currently converted
to strings internally anyway). The following options are defined:

- *max_runtime*: the value of this variable is converted as double
  and, if non-negative, used to compute the max test runtime. After such
  amount of time has passed, the test will stop running automatically.

  By default, there is no maximum runtime constraint for tests.

- *randomize_input*: the value of this variable is converted to bool
  and, if true, instructs measurement-kit to randomize the input.

  By default, input is randomized.

The `on_entry` method allows to specify the delegate called when
a test entry is about to be written to disk. The first argument
receives the entry object serialized as JSON. Note that the entry
format MUST be compatible with [OONI specification](https://github.com/TheTorProject/ooni-spec/tree/master/data-formats).

The `on_begin` and `on_end` methods allow to specify delegates called
respectively when the test is about to begin and when the test is about
to end. They may be useful to start and stop filtering for tests
related events, for example. You MAY specify multiple `on_end` callbacks
if you wish; they will be executed sequentially at end of test.

The `on_destroy` method allows to register delegates called when the
test object is about to be destroyed. These delegates will be invoked
sequentially by `~BaseTest`. This is the correct place to free the
resources allocated for the purpose of allowing other test callbacks
to run in other languages (e.g. Java and Python).

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
