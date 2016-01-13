# NAME
ooni::DnsInjectionTest -- OONI dns-injection test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

// Run sync test
mk::ooni::DnsInjectionTest()
    .set_backend("127.0.0.1")
    .set_input_file_path("test/fixtures/hosts.txt")
    .set_output_file_path("results.yml")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::DnsInjectionTest()
    .set_backend("127.0.0.1")
    .set_input_file_path("test/fixtures/hosts.txt")
    .set_output_file_path("results.yml")
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

The `DnsInjectionTest` class allows to run OONI dns-injection test. After
instantiating the class, you can configure it using these methods:

- *set_backend*: set address (and optionally port) of the host to be
  used as backend for the test. To specify the port append a colon followed
  by the port to the IPv4/IPv6 address. Note that the test assumes that no
  DNS server is running on the backend and its results rely on that.

- *set_input_file_path*: set the path to the input file, i.e., a file
  that contains the domain names to be tested, one for each line.

- *set_output_file_path*: optional method to set output file path, otherwise
  a file named "report-dns_injection-DATE.yamloo" is written into the current
  working directory.

- *set_verbose*: if called, this method tells the test to run in verbose
  mode, i.e., to produce more detailed logs of its operations. The default
  is for the test to be quiet.

- *on_log*: sets the callback called each time a log line is generated
  by the test. The default is to print logs on the stderr.

Note that you MUST call *set_backend* and *set_input_file_path* because
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

The `DnsInjectionTest` class appeared in MeasurementKit 0.1.0.
