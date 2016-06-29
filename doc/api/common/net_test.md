# NAME
NetTest &mdash; Base network test class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/common.hpp>

class NetTest : public NonCopyable, public NonMovable {
  public:
    /* Set up: */
  
    NetTest &on_log(Delegate<uint32_t, const char *>);
    NetTest &set_verbosity(uint32_t);
    NetTest &increase_verbosity();
    NetTest &set_input_file_path(std::string);
    NetTest &set_output_file_path(std::string);
    NetTest &set_reactor(Var<Reactor>);
    template <typename T> NetTest &set_options(std::string, T);

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor = Reactor::global();
    Settings options;
    std::string input_filepath;
    std::string output_filepath;

    /* Running in a background thread: */

    virtual Var<NetTest> create_test_();
    void run();
    void run(Callback<>);

    /* Running in a foreground thread: */

    virtual void begin(Callback<>);
    virtual void end(Callback<>);
};

```

# STABILITY

2 - Stable

# DESCRIPTION

`NetTest` is the abstract base class of all tests. It defines the
basic behavior of a test but certain aspects are left to subclasses
to implement. One notable subclass is `OoniTest`, which implements
all the operations required by a OONI-like test (i.e. identification
of the client IP address, ISP and location, production of a JSON
report during the test, submission of the report to a collector host.)

The `NetTest` class contains the following public attributes:

- *logger*: logger object to use
- *reactor*: reactor object to use
- *options*: options of the test
- *input_filepath*: optional path of the input file
- *output_filepath*: optional path of the output file

These attributes could either be set directly as in

```C++
    FooTest test;
    test.options["foo"] = false;
    test.output_filepath = "/tmp/file.json";
```

Or they could be set using setters leading to a domain-specific-language
style of initialization of tests, e.g.:

```C++
    FooTest test = FooTest()
        .set_options("foo", false)
        .set_output_filepath("/tmp/file.json");
```

The domain-specific-language initialization style could be combined with the
`run()` methods, allowing to express the intention of running a specific test
in a very compact and declarative way:

```C++
    FooTest()
        .set_options("foo", false)
        .set_options("/tmp/file.json")
        .run();
```

The difference between `run()` and `run(Callback<>)` is that the former is
blocking, while the latter is nonblocking and invokes the provided callback
when the test is complete. Note that `run(Callback<>)` executes the test
in a background thread, so *make sure you lock shared resources* inside the
callback; that is:

```C++
    FooTest()
        .set_options("foo", false)
        .set_options("/tmp/file.json")
        .run([=]() {
            shared_resource.lock();
            // Do something with shared resource
        });
```

Internally `run(Callback<>)` would use a `Runner` to run the test in a
background thread. What would be passed to such `Runner` is not the test
instantiated on the stack, rather a dynamically allocated copy of it
created using the `create_test_()` function that subclassess MUST therefore
override. (The default implementation of such function returns an
uninitialized `Var<NetTest>` which would throw `std::runtime_error`
when the `Runner` would try to access the underlying pointer.)

Also note that `run()` MAY just be syntactic sugar for `run(Callback<>)` and
MAY therefore execute the test in a background thread using a `Runner`. (As of
0.2.0 this is indeed how it is implemented.)

If you want more control on how the test is executed, there is a lower
level API, comprised by the `begin()` and `end()` methods; e.g.:

```C++

// Execute test using a specific reactor
void execute_with(Var<Reactor> reactor, Callback<> cb) {
    Var<NetTest> test(new FooTest);
    test->options["foo"] = false;
    test->output_filepath = "/tmp/file.json";
    test->reactor = reactor;
    test->begin([=]() {
        test->end([=]() {
            cb();
        });
    });

// Or, execute test directly in main()
int main(int argc, char **argv) {
    /* initialize ... */

    FooTest test;
    test.options["foo"] = false;
    test.output_filepath = "/tmp/file.json";

    loop_with_initial_event([&test]() {
        test.begin([&test]() {
            test.end([]() {
                // Do something, presumably break the loop
            });
        });
    });
}
```

These two methods, `begin()` and `end()` MUST be overrided by subclasses. Their
default implementation just immediately call the callback passed as argument 
to signal completion.

A common error (at least, I make it quite often) is to attempt to run a test
using the wrong `reactor` (typically, you should have used a specific reactor
but you are using the default one instead). In such case, you would see the
test starting but generating no events because the test itself is bound
to a reactor that is not running.

# CAVEATS

When you allocate `NetTest` on the stack and you execute it using the low
level `begin()` and `end()` functions, it is your responsibility to ensure
that the test object would be still alive until the final callback is
called. In most cases, the most convenient strategy is to allocate the
test on the heap using a smart pointer, as shown above.


# HISTORY

The `NetTest` class appeared in MeasurementKit 0.1.0.
