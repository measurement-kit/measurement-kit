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
        "AAAA",                    // Type of query
        "www.kernel.org",          // Name to resolve
        [](Response response) {    // Callback
            if (response.get_evdns_status() != DNS_ERR_NONE) {
                return;
            }
            /* process response */
        });
```

# DESCRIPTION

The `Query` class represents an asynchronous DNS query. You set the
type of query, the name to resolve, a callback to be called once the
result of the query (either success or error) is know.

The type of query matches closely the query types made available
by `evdns`. You can choose among the following:

- `A`: resolve IPv4 address of domain
- `REVERSE_A`: resolve domain of IPv4 address
- `AAAA`: resolve IPv6 address of domain
- `REVERSE_AAAA`: resolve domain of IPv6 address

The callback could be called immediately if an error occurs while
sending the query to the server. Use `get_evdns_status()` as shown
above to distinguish between error and successful response.

This class internally uses the `evdns_base` made available by
the global `Poller` and uses the default resolver configured in
your operating system. Use the `Resolver` class to customize
settings (e.g. to change the resolver address).

# BUGS

The interface of this class is higher level than the one of many
other DNS libraries (e.g. the one of Twisted). For example, you do
not specify the domain (typically `IN`) and you cannot say to the
code to issue a `PTR` request for an address reversed at hand by
yourself. This is a feature in terms of usability but may also be
a bug if seen from the point of view of OONI.

# HISTORY

The `Query` class appeared in MeasurementKit 0.1.
