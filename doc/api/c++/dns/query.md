# NAME
Query -- DNS async query

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <event2/dns.h>
#include <measurement_kit/dns.hpp>

using namespace measurement_kit::dns;

// Construct empty DNS query message
Query query;

// Query for www.kernel.org's IPv6 address
Query query(
        "IN",                                 // Domain of the query
        "AAAA",                               // Type of query
        "www.kernel.org",                     // Name to resolve
        [](Error error, Response response) {  // Callback
            if (error) {
                return;
            }
            /* process response */
        });
```

# DESCRIPTION

The `Query` class represents an asynchronous DNS query. You set the
type of query, the name to resolve, a callback to be called once the
result of the query (either success or error) is know.

The domain of the query must be `IN`. (Instead of specifying a string,
e.g. `"IN"`, you can also explictly specify the corresponding query class
id, e.g. `QueryClassId::IN`.)

The type of query matches closely the query types made available
by `evdns`. You can choose among the following:

- `A`: resolve IPv4 address of domain
- `REVERSE_A`: resolve domain of IPv4 address
- `AAAA`: resolve IPv6 address of domain
- `REVERSE_AAAA`: resolve domain of IPv6 address
- `PTR`: perform reverse DNS resolution

(Of course, instead of specifying the types as strings, e.g. `"A"`, you
can specify the corresponding query type, e.g. `QueryTypeId::A`.)

The difference between `REVERSE_A`, `REVERSE_AAAA` and `PTR` is that
`REVERSE_A` and `REVERSE_AAAA` take in input respectively an IPv4 and
an IPv6 address, while for `PTR` you need to construct yourself the
reversed IP address name to query.

The callback could be called immediately if an error occurs while
sending the query to the server. Use `get_evdns_status()` as shown
above to distinguish between error and successful response.

This class internally uses the `evdns_base` made available by
the global `Poller` and uses the default resolver configured in
your operating system. Use the `Resolver` class to customize
settings (e.g. to change the resolver address).

# HISTORY

The `Query` class appeared in MeasurementKit 0.1.
