# NAME
interface - Interface of the traceroute module.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/traceroute.hpp>

namespace mk {
namespace traceroute {

class Interface {
  public:
    void send_probe(std::string addr, int port, int ttl,
                    std::string payload, double timeout);
    void on_result(Callback<ProbeResult> cb);
    void on_timeout(Callback<> cb);
    void on_error(Callback<Error> cb);
};


} // namespace traceroute
} // namespace mk
```

# STABILITY

1 - Experimental

# DESCRIPTION

This is the interface of the traceroute module. Specific implementations should
implement this interface.

The `send_probe()` method sends a single UDP packet to the specified `addr`
and `port`, with the specified `ttl` and `payload` and waits up to `timeout` seconds
for a response (either valid or for a ICMP error).

The `on_result()` method allows to set the callback called when a response (either
valid or a ICMP error) is returned. The `ProbeResult` struct is like:

```C++
class ProbeResult {
  public:
    std::string interface_ip;      // Host that replied
    int ttl = 0;                   // Response TTL
    double rtt = 0.0;              // Round trip time
    bool is_ipv4 = true;           // Are we using IPv4?
    unsigned char icmp_type = 255; // Raw ICMP/ICMPv6 type
    unsigned char icmp_code = 255; // Raw ICMP/ICMPv6 code
    ssize_t recv_bytes = 0;        // Bytes recv'd
    bool valid_reply = false;      // Whether reply is valid
    std::string reply;             // Reply packet data

    ProbeResultMeaning get_meaning();
};
```

where `interface_ip` is the address of the host that send the ICMP response, `ttl`
is the response TTL, `rtt` is the time elapsed since sending the UDP packet until
receiving the response or the ICMP error, `is_ipv4` indicates whether the response
is ICMPv4 or ICMPv6, `icmp_type` and `icmp_code` are the raw ICMPv4 or ICMPv6 type
and code values received, `recv_bytes` is the number of bytes received, `valid_reply`
is `true` if we received a response and `false` if we received a ICMP error, and
`reply` contains the reply if `valid_reply` is `true`.

The `get_meaning()` method maps the raw ICMP type and code into the following
structure, thus abstracting the differences between ICMPv4 and ICMPv6:

```C++
enum class ProbeResultMeaning {
    OTHER = 0,            // Another meaning
    NO_ROUTE_TO_HOST = 1, // No route to host
    ADDRESS_UNREACH = 2,  // E.g., link down
    PROTO_NOT_IMPL = 3,   // UDP not implemented
    PORT_IS_CLOSED = 4,   // Port is closed
    TTL_EXCEEDED = 5,     // TTL is too small
    ADMIN_FILTER = 6,     // E.g., firewall rule
    GOT_REPLY_PACKET = 7, // We got a real reply packet
};
```

The `on_timeout()` method allows to set the callback called when a timeout occurs.

The `on_error()` method allows to set the callback called when a network error (distinct
from receiving a ICMP error) occurs.


# HISTORY

The `interface` submodule appeared in MeasurementKit 0.1.0.
