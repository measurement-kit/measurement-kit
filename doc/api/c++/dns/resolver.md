# NAME
Resolver -- DNS resolver

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <event2/event.h>
#include <measurement_kit/dns.hpp>

using namespace measurement_kit::dns;

// Constructs resolver with default settings
Resolver resolver;

// Constructs resolver with specific settings
Resolver reso({
    {"nameserver", "8.8.8.8:53"},  // Set the name server IP
    {"attempts", "1"},             // How many attempts before erroring out
    {"timeout", "3.1415"},         // How many seconds before timeout
    {"randomize_case", "1"},       // Whether to randomize request case
});

// Issue an async DNS query
reso.query(
        "IN",                                   // Domain of the query
        "AAAA",                                 // Type of query
        "nexa.polito.it",                       // Name to resolve
        [](Error error, Response response) {    // Callback
            if (error) {
                return;
            }
            /* handle successful response */
        });
```

# DESCRIPTION

The `Resolver` class represents a DNS resolver. When you construct
a resolver you can specify the following settings:

- *attempts*: how many attempts before erroring out (default is three)

- *nameserver*: address (and optionally port) of the name server (if you
  don't specify this, the default name server is used)

- *randomize_case*: whether to [randomize request case to make DNS
  poisoning more complex](https://lists.torproject.org/pipermail/tor-commits/2008-October/026025.html)
  (by default this is not done)

- *timeout*: time after which we stop waiting for a response (by
  default this is five seconds)

Once the resolver is constructed, you can use it to issue queries using
an interface equal to the one of the `Query` class.

# BUGS

Options can only be specified as strings. It would be nice to allow for them
to be either string or numbers, depending on their semantic.

# SEE ALSO

The documentation of the `Query` class.

# HISTORY

The `Resolver` class appeared in MeasurementKit 0.1.
