# NAME
ooni::HttpInvalidRequestLineTest -- OONI http-invalid-request-line test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

// Run sync test
mk::ooni::HttpInvalidRequestLineTest()
    .set_backend("http://127.0.0.1/")
    .set_output_file_path("results.yml")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::HttpInvalidRequestLineTest()
    .set_backend("http://127.0.0.1/")
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

The `HttpInvalidRequestLineTest` class allows to run OONI
http-invalid-request-line test. After instantiating the class,
you can configure it using these methods:

- *set_backend*: set URL of the backend, i.e., of the helper server where
  an echo server, required by the test, is running.

- *set_output_file_path*: optional method to set output file path, otherwise
  a file named "report-http_invalid_request_line-DATE.json" is written into
  the current working directory.

- *set_verbose*: if called, this method tells the test to run in verbose
  mode, i.e., to produce more detailed logs of its operations. The default
  is for the test to be quiet.

- *on_log*: sets the callback called each time a log line is generated
  by the test. The default is to print logs on the stderr.

Note that you MUST call *set_backend* because it provides parameters required
by the test.

Once you are satisfied with the test configuration, call the *run* method
to start the test. If you do not pass any argument to *run*, the test is
run synchronously and *run* only returns when the test is complete. If you
provide a callback as the first argument, the test is run in a background
thread, and the callback is called when the test is complete.

# CAVEATS

Callbacks MAY be called from a background thread. It is your responsibility
to acquire the proper locks before manipulating shared objects.

# HISTORY

The `HttpInvalidRequestLine` class appeared in MeasurementKit 0.1.0.
