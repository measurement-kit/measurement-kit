# NAME
ndt &mdash; Network Diagnostic Tool client

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {

class NdtTest : public NetTest {
  public:
    using NetTest::NetTest;
};

} // namespace ndt
} // namespace mk

void mk::ndt::run(Callback<Error> callback,
                  Settings settings = {},
                  Var<Logger> logger = Logger::global(),
                  Var<Reactor> reactor = Reactor::global());

void mk::ndt::run_with_specific_server(std::string address,
                                       int port,
                                       Callback<Error> callback,
                                       Settings settings = {},
                                       Var<Logger> logger = Logger::global(),
                                       Var<Reactor> reactor = Reactor::global());
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `NdtTest` class is a subclass of `mk::NetTest` that runs a NDT test. Hence you
can run a NDT test as follows:

```C++
#include <measurement_kit/ndt.hpp>

using namespace mk::ndt;

NdtTest()
    .set_options("test_suite", MK_NDT_DOWNLOAD)
    .increase_verbosity()
    .run();
```

More options than the one displayed above are available; see the documentation of
`mk::NetTest` for more information on how to run a NetTest and on what options are
available.

The `run()` function runs a NDT test and calls the `callback` specified as its first
argument when done, passing it the error that occurred &mdash; or `NoError()` in case
of success. The behavior of the function may be futher specified by passing it the
following `settings`:

- *"address"*: address (or domain name) of the NDT server. If this argument is not
  specified, mlabns is used to find out a suitable server.

- *"port"*: port of the NDT server as integer. Only used if *address* is specified.

- *"test_suite"*: integer mask indicating which tests to run among all the available
  tests as defined by the following `define`s:

```C++
#define MK_NDT_UPLOAD 2    // Run NDT's download test
#define MK_NDT_DOWNLOAD 4  // Run NDT's upload test
```

If you don't specify a *test_suite* setting, both the download and the upload NDT
tests will be run. Additionally, you can also specify an optional `reactor` and
an optional `logger`.

The `run_with_specific_server()` function is a wrapper that sets *address* and *port*
in its settings according to its first two arguments and then calls `run()`. All other
arguments have the same semantic as their `run()` equivalents.

# EXAMPLE

Runs NDT test in the context of the default runner and breaks out of the event loop
when the NDT test is complete, whatever the test result.

```C++
#include <measurement_kit/ndt.hpp>

using namespace mk;

mk::loop_with_initial_event([]() {
    ndt::run([](Error /*err*/) { break_loop(); });
});
```

You can also achieve the same results more compactly and in a more abstract way by using
the `mk::NetTest` interface as shown above.

# BUGS

Only NDT's download and upload tests are implemented.

# HISTORY

The `ndt` module appeared in MeasurementKit 0.2.0.
