# NAME
Defines -- Define DNS query classes and types

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/dns.hpp>

mk::dns::QueryClassId::IN;           // internet class

mk::dns::QueryTypeId:A;              // name to IPv4 addresses
mk::dns::QueryTypeId:REVERSE_A;      // IPv4 address to one or more names
mk::dns::QueryTypeId:AAAA;           // name to IPv6 addresses
mk::dns::QueryTypeId:REVERSE_AAAA;   // IPv6 address to one or more names
mk::dns::QueryTypeId:PTR;            // `.arpa` name to name

mk::dns::QueryClass c(mk::dns::queryClassId::IN);  // construct from id
mk::dns::QueryClass cc("IN");             // construct from string
assert(c == cc);
c == mk::dns::QueryClassId::IN;           // equality
c != mk::dns::QueryClassId::CH;           // unequality
mk::dns::QueryClassId x = cc;             // automatic cast

mk::dns::QueryType q(mk::dns::QueryTypeId::A);     // construct from id
mk::dns::QueryType qq("A");               // construct from string
assert(q == qq);
q == mk::dns::QueryTypeId:A;              // equality
q != mk::dns::QueryTypeId:MX;             // unequality
mk::dns::QueryTypeId x = cc;              // automatic cast

```

# DESCRIPTION

These enum classes define the possible values that you can use for
making DNS queries using `Query` and `Resolver`.

For convenience, it is possible to use strings rather than enums. That is,
`QueryClass("IN")` is equal to `QueryClass(QueryClassId::IN)`.

We define more classes than we support. Currently we only support `IN`
(i.e. internet).

We also define more types than we support. See the documentation of
`Query` for an updated list of the types we support.

The `REVERSE_A` and `REVERSE_AAAA` are non standard query types that
perform for you PTR requests assembling for you the correct domain names
in the `.in-addr.arpa` or `ip6.arpa` namespaces.

# HISTORY

The `QueryTypeId`, `QueryType`, `QueryClassId`, and `QueryClass` classes
appeared in MeasurementKit 0.1.0.
