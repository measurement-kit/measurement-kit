# NAME
Resolver -- DNS resolver

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/dns.hpp>

// Constructs resolver with default settings
mk::dns::Resolver resolver;

// Constructs resolver with specific settings
mk::dns::Resolver reso({
    {"nameserver", "8.8.8.8:53"},  // Set the name server IP
    {"attempts", "1"},             // How many attempts before erroring out
    {"timeout", "3.1415"},         // How many seconds before timeout
    {"randomize_case", "1"},       // Whether to randomize request case
});

// Issue an async DNS query
reso.query(
        "IN",                                             // Domain of the query
        "AAAA",                                           // Type of query
        "nexa.polito.it",                                 // Name to resolve
        [](mk::Error error, mk::dns::Response response) { // Callback
            if (error) throw error;
            // handle successful response
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

Once the resolver is constructed, you can use it to issue queries
using its `query()` method. You pass to this method the
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
sending the query to the server.

# BUGS

Options can only be specified as strings. It would be nice to allow for them
to be either string or numbers, depending on their semantic.

# HISTORY

The `Resolver` class appeared in MeasurementKit 0.1.0.
