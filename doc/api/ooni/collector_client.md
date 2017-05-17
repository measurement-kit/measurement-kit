# NAME
collector_client -- Routines to interact with OONI's collector

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

std::string testing_collector_url();
std::string production_collector_url();

void mk::ooni::submit_report(std::string filepath, std::string collector_base_url,
        Callback<Error> callback, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

void mk::ooni::submit_report(std::string filepath, std::string collector_base_url,
        std::string collector_front_domain,
        Callback<Error> callback, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

void mk::ooni::connect(Settings settings, Callback<Error, Var<net::Transport>> callback,
        Var<Reactor> reactor = Reactor::global(), Var<Logger> logger = Logger::global());

void mk::ooni::create_report(Var<net::Transport> txp, report::Entry entry,
        Callback<Error, std::string> callback, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

void mk::ooni::update_report(Var<net::Transport> txp, std::string report_id,
        report::Entry, entry, Callback<Error> callback, Settings settings = {},
        Var<Reactor> = Reactor::global(),
        Var<Logger> = Logger::global());

void mk::ooni::close_report(Var<net::Transport> txp, std::string report_id,
        Callback<Error> callback,
        Settings settings = {}, Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());
```

# STABILITY

2 - Stable

# DESCRIPTION

This header contains routines to interact with OONI's collector.

The `testing_collector_url()` function returns the testing collector URL,
and the `production_collector_url()` returns the production one.

The `submit_report()` function submits the report at `filepath` using the collector
identifier by `collector_base_url` and calls `callback` when done. You can also
optionally specify `settings`, a `reactor`, and a `logger`.
It is possible to submit reports via cloudfronting by specifying
`collector_front_domain`. In this case a connection will be opened to
`collector_front_domain` and the `Host` header will be set to point to the domain
contained inside of `collector_base_url`.

For example if you want to use cloudfronting via `a0.awsstatic.com` to
speak to the canonical ooni bouncer (`https://das0y2z2ribx3.cloudfront.net`),
you should do:

```
collector::submit_report("/path/to/report", "https://das0y2z2ribx3.cloudfront.net",
    "a0.awsstatic.com", [=](Error err) {
            // Handle errors
});
```

The `connect()` function connects to the address specified by the *"collector_base_url*"
setting and calls `callback` when done. Optional `reactor` and `logger` could also be
passed to this function.

The `create_report()` function takes in input the first entry of a report and a
transport connected to a collector, and creates the corresponding report on
the collector. The `callback` function is called when done. Optional `settings`,
`reactor` and `logger` can be specified. Beware that this function only creates the
report but does not submit the actual `entry` to the report; only the specific
metadata contained in `entry` would be used and submitted by this function. The
callback returns as first argument the error that occureed &mdash; or `NoError()`
&mdash; and as second argument the report-id identifying the report.

The `update_report()` function takes in input a `entry`, a `txp` transport
connected to a collector and a `report_id`, and updates the corresponding report
by submitting the given report entry. The `callback` function is called when
done. Optional `settings`, `reactor` and `logger` can be specified.

The `close_report()` function takes in input a `txp` transport
connected to a collector and a `report_id`, and closes the corresponding
report. The `callback` function is called when
done. Optional `settings`, `reactor` and `logger` can be specified.

# HISTORY

The `collector_client` module appeared in MeasurementKit 0.2.0.
