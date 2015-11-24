# NAME
ooni::TcpConnectTest -- OONI tcp-connect test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

using namespace measurement_kit;

// Run sync test
ooni::TcpConnectTest()
    .set_port("80")
    .set_input_file_path("test/fixtures/hosts.txt")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
ooni::TcpConnectTest()
    .set_port("80")
    .set_input_file_path("test/fixtures/hosts.txt")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run([]() {
        // If needed, acquire the proper locks
        // Handle test completion
    });
```

# DESCRIPTION

The `TcpConnectTest` class allows to run OONI tcp-connect test. After
instantiating the class, you can configure it using these methods:

- *set_port*: set port to connect to during the test.

- *set_input_file_path*: set the path to the input file, i.e., a file
  that contains the domain names to be tested, one for each line.

- *set_verbose*: if called, this method tells the test to run in verbose
  mode, i.e., to produce more detailed logs of its operations. The default
  is for the test to be quiet.

- *on_log*: sets the callback called each time a log line is generated
  by the test. The default is to print logs on the stderr.

Note that you MUST call *set_port* and *set_input_file_path* because
they provide parameters required by the test.

Once you are satisfied with the test configuration, call the *run* method
to start the test. If you do not pass any argument to *run*, the test is
run synchronously and *run* only returns when the test is complete. If you
provide a callback as the first argument, the test is run in a background
thread, and the callback is called when the test is complete.

# CAVEATS

Callbacks MAY be called from a background thread. It is your responsibility
to acquire the proper locks before manipulating shared objects.

# HISTORY

The `TcpConnect` class appeared in MeasurementKit 0.1.0.
