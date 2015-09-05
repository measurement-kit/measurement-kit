# NAME
Response -- DNS response message

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <event2/dns.h>
#include <measurement_kit/dns.hpp>

using namespace measurement_kit::dns;

Response response;        // by default evdns status is DNS_ERR_UNKNOWN

Response resp(
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

// Get evdns type
char type = resp.get_type();
if (type == DNS_IPv4_A) {
    /* do something if request was for IPv4 */
}
```

# DESCRIPTION

The `Response` class holds the result of an async DNS `Query`. You do not
typically instantiate a `Response` yourself, rather you receive an instance
of `Response` in response to a DNS query.

# BUGS

Since `evnds` does not report whether the response was authoritative, the
`is_authoritative()` method always returns `"unknown"`.

# SEE ALSO

For a list of `evdns` status codes and a list of evdns types, please refer
to the [evdns implementation](https://github.com/libevent/libevent/blob/master/include/event2/dns.h).

# HISTORY

The `Response` class appeared in MeasurementKit 0.1.
