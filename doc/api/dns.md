# NAME
dns &mdash; DNS module

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/dns.hpp>

void mk::dns::query(mk::dns::QueryClass dns_class,
                    mk::dns::QueryType dns_type,
                    std::string query_name,
                    mk::Callback<mk::Error, mk::dns::Message> callback,
                    mk::Settings settings = {},
                    mk::Var<mk::Reactor> reactor = mk::Reactor::global());
```

# STABILITY

2 - Stable

# DESCRIPTION

The `query()` function allows you to send DNS queries
and receive the corresponding responses.

The `dns_class` argument indicates the query class. At least
the following query classes are defined:

- *QueryClassId::IN*: this class represents the "internet" domain

Note that you can also pass the query class as string; e.g.,
the following would compile and run as expected:

```C++
    mk::dns::query("IN", ...);
```

The `dns_type` argument indicates the query type. The following
query types are defined:

- *QueryTypeId::A*: the `query_name` argument must be a domain name and the result
  would be the corresponding IPv4 address, if any.

- *QueryTypeId::AAAA*: the `query_name` argument must be a domain name and the result
  would be the corresponding IPv6 address. if any.

- *QueryTypeId::PTR*: the `query_name` argument should be an IP address expressed
  using the reverse `IN-ADDR` representation and the result would the corresponding
  domain name, if any (see `EXAMPLES` section for examples).

- *QueryTypeId::REVERSE_A*: the `query_name` argument should be an IPv4 address and the
  result would be the corresponding domain name, if any. This is a nonstandard
  DNS query type and basically instructs the DNS library to create for you
  the reverse `IN-ADDR` representation of the `query_name` field and issue a `PTR`
  query.

- *QueryTypeId::REVERSE_AAAA*: same as `REVERSE_A` except that here the input shall be
  a IPv6 address.

Note that you can also pass the query type as string; e.g. the following
would also work as expected:

```C++
    mk::dns::query("IN", "A", "www.google.com", ...);
```

The `callback` argument is a lambda to be called when the DNS response is available
or an error occurs. In case of success, error would be equal to `NoError()`. Otherwise,
the error that occurred is reported. Among all the possible errors, the following are
defined by MeasurementKit DNS implementation:

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

In case of success, the `Message` argument passed to the callback would
contain details on the response. The `Message` structure contains at least
the following fields:

```C++
class Message {
  public:
    double rtt = 0.0;
    std::vector<Query> queries;
    std::vector<Answer> answers;
};
```

where `rtt` is the time elapsed since issuing the query until receiving
the response; `queries` is the list of queries issued; `answers` is the list
of answers received.

The `Query` class contains at least the following fields:

```C++
class Query {
  public:
    QueryType type;
    QueryClass qclass;
    uint32_t ttl = 0;
    std::string name;
};
```

where `type` is the type of the query, `qclass` is the class of the
query, `ttl` is the time to live, and `name` is the name for which the
query was issued.

The `Answer` class contains at least the following fields:

```C++
class Answer {
  public:
    QueryType type;
    QueryClass qclass;
    uint32_t ttl = 0;
    std::string ipv4;             // For A records
    std::string ipv6;             // For AAAA records
    std::string hostname;         // For PTR, SOA and CNAME records
    std::string responsible_name; // For SOA records
    uint32_t serial_number;       // For SOA records
    uint32_t refresh_interval;    // For SOA records
    uint32_t retry_interval;      // For SOA records
    uint32_t minimum_ttl;         // For SOA records
    uint32_t expiration_limit;    // For SOA records
};
```

where `type` and `qclass` represent respectively the query type and the
query class, `ttl` is the response time to live, and the following fields
are only set for specific query types.

The optional `Settings` argument contains settings modifying the behavior of
the `query` function. The following setting keys are available:

- *"dns/attempts"*: how many attempts before erroring out (default is three)

- *"dns/nameserver"*: address (and optionally port) of the name server to use. If you
  don't specify this, the default name server is used. On Unix systems the default DNS
  server is obtained parsing `/etc/resolv.conf`; on mobile devices where such file
  is not available, the default DNS name server is `127.0.0.1` which typically is not
  correct. Hence with mobile devices you SHOULD typically supply the DNS server
  you would like to use.

- *"dns/randomize_case"*: whether to [randomize request case to make DNS
  poisoning more complex](https://lists.torproject.org/pipermail/tor-commits/2008-October/026025.html)
  (by default this is not done)

- *"dns/timeout"*: time after which we stop waiting for a response (by
  default this is five seconds)

The optional `reactor` argument is the reactor to use to issue the query
and receive the corresponding response.

# EXAMPLE

```C++
#include <measurement_kit/dns.hpp>

using namespace mk;

Settings settings({
    {"dns/nameserver", "8.8.8.8:53"},
    {"dns/attempts", 1},
    {"dns/timeout", 3.1415},
    {"dns/randomize_case", true},
});

dns::query(
        "IN", "AAAA", "nexa.polito.it",
        [](Error error, dns::Message message) {
            if (error) {
                throw error;
            }
            double rtt = message.rtt;
            for (auto answer : message.answers) {
                int ttl = answer.ttl;
                std::string r;
                if (answer.type == "A") {
                    r = answer.ipv4;
                } else if (answer.type == "AAAA") {
                    r = answer.ipv6;
                } else if (answer.type == "PTR") {
                    r = answer.hostname;
                } else {
                    continue;
                }
                /* ... */
            }
        }, settings);
```

# HISTORY

The DNS module appeared in MeasurementKit 0.1.0.
