# NAME
utils &mdash; Utility functions

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class Endpoint {
  public:
    std::string hostname;
    uint16_t port = 0;
};

bool is_ipv4_addr(std::string s);
bool is_ipv6_addr(std::string s);
bool is_ip_addr(std::string s);

ErrorOr<Endpoint> parse_endpoint(std::string s, uint16_t default_port);
std::string serialize_endpoint(Endpoint e);

Error make_sockaddr(
        std::string address,
        uint16_t port,
        sockaddr_storage *ss,
        socklen_t *len
) noexcept;

}}
```

# DESCRIPTION

The `is_ipv4_addr`, `is_ipv6_addr`, `is_ip_addr` functions return true
if the input is, respectively, IPv4, IPv6 and IPv4 or IPv6.

The `parse_endpoint` function parses the string `s` &mdash; which should
contain an IP address or an hostname, followed by a colon and by a port
&mdash; into an `Endpoint` structure. The return value is an `ErrorOr`
template wrapping the `Endpoint` structure on success and a `ValueError`
error if the parsing fails. This function follows this algorithm:

1. parse `s` as `host ":" port` where `host` and `port` are defined
as defined by the HTTP standards, returns the `Endpoint` on success, and
`ValueError` on failure

2. if step 1 fails, appends `:` followed by `default_port` to `s` and
calls the functionality at step 1 again with the resulting string

The `host` MAY either be a domain name, or an IPv4 or IPv6 address. If
`host` is an IPv6 address and `s` also contains the port, the `host` MUST
be quoted using `[` and `]` as in `[::1]`. If `s` only contains an IPv6
address, then the square brackets MAY be omitted.

The `serialize_endpoint` function transforms the endpoint `e` into
a string. As such, it is the dual operation of `parse_endpoint`.

The `make_sockaddr` functions fills a `sockaddr_storage` structure and its
length from a string possibly containing an IPv4 or IPv6 address and a
port (either as a string or as an integer). The return value is `NoError`
on success and an error on failure.

# BUGS

In theory, `parse_endpoint` should correctly deal with link scope
addresses such as `[fe80::1%lo0]:80`. However, it was only possible
to test this functionality with macOS, therefore it MAY NOT work
correctly on Linux systems.

# HISTORY

Module added in MeasurementKit 0.4.0.
