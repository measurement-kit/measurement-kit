# NAME
mlabns &mdash; module for querying mlab name service (mlabns)

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/mlabns.hpp>

void mk::mlabns::query(std::string mlab_tool,
                       mk::Callback<mk::Error, mk::mlabns::Reply> callback,
                       mk::Settings settings = {},
                       mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                       mk::Var<mk::Logger> logger = mk::Logger::global());
```

# STABILITY

2 - Stable

# DESCRIPTION

The `query` function allows to query the mlab naming service (mlabns). The first argument
is the tool for which you are querying mlabns (e.g. `ndt`). The second argument is a callback
function called when done, with an error &mdash; or `NoError()` in case of success &mdash;
passed as the first argument and the reply passed as second argument. You can also pass
the following, optional settings:

- *"mlabns/policy"*: one of `"random"`, `"metro"`, or `"country`". The random policy asks mlabns
  to return a random server for the specified tool. The metro policy asks mlabns a server close to
  the city code passed as *mlabns/metro*. The country policy asks mlabns to return a suitable server
  in the country where the requesting client is located.

- *"mlabns/metro"*: used together with *mlabns/policy* equal to *metro*, this setting specifies
  which city should the returned server be close to. For example, *ath* (for Athens) or *trn* (for
  Turin).

- *"mlabns/family"*: when this setting is unspecified, mlabns returns results valid for both IPv4
  and IPv6. Set *mlabns/family* to *ipv4* or *ipv6* to restrict the results respectively only
  to IPv4 or IPv6.

Optionally you can also pass a nondefault `reactor` and `logger`.

The `Reply` structure is like:

```C++
class Reply {
  public:
    std::string city;            // City where server is.
    std::string url;             // URL to access server using HTTP.
    std::vector<std::string> ip; // List of IP addresses of server.
    std::string fqdn;            // FQDN of server.
    std::string site;            // Code of the city where the server is (e.g. `ath`).
    std::string country;         // Country where sliver is.
};
```

In addition to `NoError()`, the following errors could be returned by the callback:

- *mk::mlabns::InvalidPolicyError*: you passed in an invalid policy setting
- *mk::mlabns::InvalidAddressFamilyError*: you passed in an invalid address family setting
- *mk::mlabns::InvalidMetroError*: you passed in an invalid metro setting
- *mk::mlabns::InvalidToolNameError*: you passed in an invalid tool name

# EXAMPLE

```C++
#include <iostream>
#include <measurement_kit/mlabns.hpp>

using namespace mk;

mlabns::query(
        "ndt", [](Error error, mlabns::Reply reply) {
            if (error) {
                throw error;
            }
            std::cout << "< city: " << reply.city << "\n";
            std::cout << "< url: " << reply.url << "\n";
            std::cout << "< ip: [\n";
            for (auto s : reply.ip) {
                std::cout << "<  " << s << "\n";
            }
            std::cout << "< ]\n";
            std::cout << "< fqdn: " << reply.fqdn << "\n";
            std::cout << "< site: " << reply.site << "\n";
            std::cout << "< country: " << reply.country << "\n";
            /* ... */
        },
        {
            {"mlabns/metro", "ath"},
            {"mlabns/policy", "metro"},
            {"mlabns/family", "ipv4"},
        });
```

# HISTORY

The `mlabns` module appeared in MeasurementKit 0.2.0.
