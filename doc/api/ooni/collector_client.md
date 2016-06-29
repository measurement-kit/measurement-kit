# NAME
collector_client -- Routines to interact with OONI's collector

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

std::string default_collector_url();

void mk::ooni::submit_report(std::string filepath, std::string collector_base_url,
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

The `default_collector_url()` function returns the default collector URL.

The `submit_report()` function submits the report at `filepath` using the collector
identifier by `collector_base_url` and calls `callback` when done. You can also
optionally specify `settings`, a `reactor`, and a `logger`.

The other functions in this module (as shown above) allow finer grained control
over the interaction with the backend. They are currently undocumented.

# HISTORY

The `collector_client` module appeared in MeasurementKit 0.2.0.
