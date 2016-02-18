# NAME
mk::dns -- MeasurementKit DNS library

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

// Construct response with default parameter
mk::dns::Response response; // by default evdns status is DNS_ERR_UNKNOWN

mk::dns::Response resp(
        DNS_ERR_NONE,     // evdns status code
        DNS_TYPE_IPv4,    // evdns type
        n_records,        // number of returned records
        ttl,              // records TTL
        started,          // time when request was started (double)
        addresses);       // opaque response body

// Get resolved records list
for (std::string s : resp.get_results()) {
    std::cout << s << "\n";
}

// Returns whether the response is authoritative
std::string authoritative = resp.is_authoritative();

// Return evdns status (this is mainly used internally)
int code = resp.get_evdns_status();

// Get time to live
int ttl = resp.get_ttl();

// Get round trip time
double rtt = resp.get_rtt();

#include <event2/dns.h> // for DNS_IPv4_A

// Get evdns type
char type = resp.get_type();
if (type == DNS_IPv4_A) {
    // do something if request was for IPv4
}
```

# DESCRIPTION

The DNS library allows you to issue DNS query and
to receive the corresponding responses.

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

(Of course, instead of specifying the types as strings, e.g. `"A"` or `AAAA`, you
can specify the corresponding query type, e.g. `QueryTypeId::A` or `QueryTypeId::AAAA`.)

The difference between `REVERSE_A`, `REVERSE_AAAA` and `PTR` is that
`REVERSE_A` and `REVERSE_AAAA` take in input respectively an IPv4 and
an IPv6 address, while for `PTR` you need to construct yourself the
reversed IP address name to query.

In case of success, the error argument of the callback is passed an
instance of `mk::NoError()`. Otherwise, the error that occurred is
reported. Among all the possible errors, the following are defined by
MeasurementKit DNS implementation:


- `FormatError`: invalid response format
- `ServerFailedError`:  server failure
- `NotExistError`:  the name does not exist
- `NotImplementedError`:  query not implemented
- `RefusedError`:  the server refuses to reply
- `TruncatedError`:  response truncated
- `UknownError`:  internal evnds error
- `TimeoutError`:  query timed out
- `ShutdownError`:  evdns library was shut down
- `CancelError`:  user cancelled query
- `NoDataError`:  no data in the response


The `Response` class holds the result of an async DNS `Query`. You do not
typically instantiate a `Response` yourself, rather you receive an instance
of `Response` in response to a DNS query.


# BUGS

Since `evnds` does not report whether the response was authoritative, the
`is_authoritative()` method of `Response` class always returns `"unknown"`.

# SEE ALSO

For a list of `evdns` status codes and a list of evdns types, please refer
to the [evdns implementation](https://github.com/libevent/libevent/blob/master/include/event2/dns.h).

# HISTORY

The `mk::dns` library appeared in MeasurementKit 0.1.0.
