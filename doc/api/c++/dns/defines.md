# NAME
Defines -- Define DNS query classes and types

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/dns.hpp>

using namespace mk::dns;

QueryClassId::IN;           // internet class

QueryTypeId:A;              // name to IPv4 addresses
QueryTypeId:REVERSE_A;      // IPv4 address to one or more names
QueryTypeId:AAAA;           // name to IPv6 addresses
QueryTypeId:REVERSE_AAAA;   // IPv6 address to one or more names
QueryTypeId:PTR;            // `.arpa` name to name

QueryClass c(queryClassId::IN);  // construct from id
QueryClass cc("IN");             // construct from string
assert(c == cc);
c == QueryClassId::IN;           // equality
c != QueryClassId::CH;           // unequality
QueryClassId x = cc;             // automatic cast

QueryType q(QueryTypeId::A);     // construct from id
QueryType qq("A");               // construct from string
assert(q == qq);
q == QueryTypeId:A;              // equality
q != QueryTypeId:MX;             // unequality
QueryTypeId x = cc;              // automatic cast

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
appeared in MeasurementKit 0.1.
