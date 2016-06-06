# NAME
NetTest -- Base network test class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/common.hpp>

class NetTest : public NonCopyable, public NonMovable {
  public:
    NetTest(Settings options);
    virtual ~NetTest();

    void on_log(Delegate<uint32_t, const char *> func);
    void set_verbosity(uint32_t);

    virtual void begin(Callback<>) = 0;
    virtual void end(Callback<>) = 0;

    unsigned long long identifier();

    Var<Logger> logger = Logger::global();
    Var<Reactor> reactor = Reactor::global();
    Settings options;
};

```

# STABILITY

1 - Experimental

# DESCRIPTION

`NetTest` is the abstract base class of all tests. It defines the
basic behavior of a test but certain aspects are left to subclasses
to implement.

The constructor initializes the public `options` attribute. The
destructor is empty and declared virtual because this is an abstract
class.

The `on_log()` and `set_verbosity()` methods allow to configure the
public `logger` attribute. The thread safety caveats that apply to
`Logger` of course also apply to these two methods.

The `begin()` and `end()` abstract methods respectively start and
conclude a test. They are not implemented and shall be implemented
by subclasses.

The `identifier()` method returns a identifier unique to this test.

# CAVEATS

It is the programmer's responsibility to make sure that the `NetTest`
(or derived class) object does not exit scope until the callback passed
to the `end()` function has been called. This can easily be achieved
using a smart pointer and passing the `NetTest` to the `Runner` class that
is designed to correctly manage the lifecycle of a test, e.g.:

```C++
    Var<NetTest> test(new SpecificTest(settings));
    Runner::global()->run(test, [=]() {
        warn("test complete");
        // Do something else here...
    });
```

# EXAMPLE

The following example creates OONI's dns_injection test on the heap
and runs it synchronously using the default runner:


```C++
#include <measurement_kit/common.hpp>

using namespace mk;

int main() {
    // XXX would be useful to unify DSL and test...
    Runner::global()->run(ooni::DnsInjectionTest::make({
        {"backend", "8.8.8.1:53"},
        {"dns/attempts", 1},
        {"dns/timeout", 0.5},
    });
}
    
    // FIXME

mk::NetTest *test = new mk::foo::Test();  // NetTest is abstract

// Configure the logger contained by NetTest
test->on_log([](const char *s) {
    // Do something
});
test->set_verbose(1);

// Get unique identifier of this test
unsigned long long identifier = test->identifier();

// Start test, tell to store results when done, tell to do something
// else when the results have been saved
test->begin([test]() {
    test->end([test]() {
        something_else(test);
    });
});
```

# BUGS

Since `Async` do not contain a mechanism for storing logs in a thread
safe way, you typically use `on_log()` to get logs and store them somewhere
within your app. Beware that the function you set with `on_log()` will be
called from the `Async` background thread context. You are responsbile
of doing the locking for yourself. (This may as well be a bug of `Async`
but mentioning it here to warn people using this interface.)

# HISTORY

The `NetTest` class appeared in MeasurementKit 0.1.0.
