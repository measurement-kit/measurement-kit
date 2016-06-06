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

# HISTORY

The `NetTest` class appeared in MeasurementKit 0.1.0.
