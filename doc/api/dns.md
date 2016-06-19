# NAME
mk::dns -- MeasurementKit DNS library

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
// TODO
```

# DESCRIPTION

The DNS library allows you to issue DNS query and
to receive the corresponding responses.

The `Settings` passed to the query function can be the following:

- *attempts*: how many attempts before erroring out (default is three)

- *nameserver*: address (and optionally port) of the name server (if you
  don't specify this, the default name server is used)

- *randomize_case*: whether to [randomize request case to make DNS
  poisoning more complex](https://lists.torproject.org/pipermail/tor-commits/2008-October/026025.html)
  (by default this is not done)

- *timeout*: time after which we stop waiting for a response (by
  default this is five seconds)

You pass the `query()` function the type of query, the name to resolve, a
callback to be called once the result of the query (either success or error) is
know.

The domain of the query must be `IN`. (Instead of specifying a string,
e.g. `"IN"`, you can also explictly specify the corresponding query class
id, e.g. `dns::QueryClassId::IN`.)

The type of query matches closely the query types made available
by `evdns`. You can choose among the following:

- `"A"`: resolve IPv4 address of domain
- `"REVERSE_A"`: resolve domain of IPv4 address
- `"AAAA"`: resolve IPv6 address of domain
- `"REVERSE_AAAA"`: resolve domain of IPv6 address
- `"PTR"`: perform reverse DNS resolution

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

# EXAMPLE

```C++
#include <measurement_kit/dns.hpp>

// Constructs settings to be passed to the query function
mk::Settings settings({
    {"dns/nameserver", "8.8.8.8:53"},  // Set the name server IP
    {"dns/attempts", "1"},             // How many attempts before erroring out
    {"dns/timeout", "3.1415"},         // How many seconds before timeout
    {"dns/randomize_case", "1"},       // Whether to randomize request case
});

// Issue an async DNS query
mk::dns::query(
        "IN",                                             // Domain of the query
        "AAAA",                                           // Type of query
        "nexa.polito.it",                                 // Name to resolve
        [](mk::Error error, mk::dns::Message message) { // Callback
            if (error) throw error;

            // Return evdns status (this is mainly used internally)
            int error_code = message.error_code;

            // Get round trip time of the query
            double rtt = message.rtt;

            for (auto answer : message.answers) {
                // Get time to live of the answer
                int ttl = answer.ttl;

                if (answer.type == dns::QueryTypeId::A) {
                    // Get the IPv4 address in the case of A answers
                    std::string ipv4 = answer.ipv4;
                } else if (answer.type == dns::QueryTypeId::AAAA) {
                    // Get the IPv6 address in the case of AAAA answers
                    std::string ipv6 = answer.ipv6;
                } else if (answer.type == dns::QueryTypeId::PTR) {
                    // Get the domain pointer in the case of PTR answers
                    std::string hostname = answer.hostname;
                }
            }
        }, settings);
```

# SEE ALSO

For a list of `evdns` status codes and a list of evdns types, please refer
to the [evdns implementation](https://github.com/libevent/libevent/blob/master/include/event2/dns.h).

# HISTORY

The `mk::dns` library appeared in MeasurementKit 0.1.0.
