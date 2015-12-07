# NAME
Query -- DNS async query

# SYNOPSIS
```C++
#include <event2/dns.h>
#include "src/dns/query.hpp"

using namespace mk::dns;
using namespace mk;

// Construct empty DNS query message
Query query;

// Query for nexa.polito.it's IPv6 address
Query query(
    "IN",                                // Domain of the query
    "AAAA",                              // Type of query
    "nexa.polito.it",                    // Name to resolve
    [](Error error, Response response) { // Callback
        if (error) throw error;
        // handle successful response
    });
```

# DESCRIPTION

The `Query` class represents an asynchronous DNS query. You set the
domain of the query, the type of query, the name to resolve, a callback
to be called once the result of the query (either success or error)
is know.

See the documentation of the DNS Resolver for available domains and types.

The callback could be called immediately if an error occurs while
sending the query to the server. Check the error as shown above to
understand whether the query was successful or not.

This class internally uses the `evdns_base` made available by
the global `Poller` and uses the default resolver configured in
your operating system. Use the `Resolver` class to customize
settings (e.g. to change the resolver address).

# HISTORY

The `Query` class appeared in MeasurementKit 0.1.0.
