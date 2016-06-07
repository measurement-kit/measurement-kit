Welcome to Measurement Kit **v0.2.0-alpha.1** documentation!

# How to generate documentation

We use [mkdocs](http://www.mkdocs.org/) to generate HTML documentation
from markdown files. To regenerate documentation, you need to have a
recent version of Python installed. Specifically, the following commands
shows how you can generate the HTML files from the markdown. You must
run them from the toplevel directory (i.e. the one that contains the
`AUTHORS` file):

```
virtualenv __venv__
source __venv__/bin/activate
pip install mkdocs
mkdocs build
```

# Public API

This documents the API defined by Measurement Kit public headers. This is
the API to which you are insterested as a Measurement Kit user.

Measurement Kit is composed of the following modules:

- *common*: contains common code
- *dns*: contains DNS code
- *http*: contains HTTP code
- *net*: contains socket-level code
- *ooni*: contains OONI tests
- *report*: contains code to generate test reports
- *tor*: contains code to control a Tor client
- *traceroute*: contains traceroute code

Each module is contained in its own header file named after the module and
stored inside the `measurement_kit` directory in the include path.

Therefore, to pull `dns` module's definitions, write:

```C++
#include <measurement_kit/dns.hpp>
```

All MeasurementKit code is contained in the `mk` namespace. There are
child namespaces for each module except `common`. In fact, definitions
contained in `common.hpp` are so common that they live in the `mk`
namespace alone to save the user a few types. For example, to use the
`Error` class of the `common` module, type:

```C++
#include <measurement_kit/common.hpp>

// ...

mk::Error error;
```

(In general, you don't need to include `common.hpp` directly, because any
other Measurement Kit header includes `common.hpp`.)

For all other submodules, the definitions live inside a sub-namespace
named after the module. So, for example, to access class `Buffer` in
the `net` module, you need to type:

```C++
#include <measurement_kit/net.hpp>

// ...

mk::net::Buffer buffer;
```

Of course, to write less in C++ files, you can use a `using namespace`
declaration to pull everything from the `mk` namespace. For example:

```C++
#include <measurement_kit/net.hpp>

using namespace mk;

// ...

Error error;
net::Buffer buffer;
```

The above example also shows that `Error` (and other `common.hpp` definitions)
are automatically pulled by `net.hpp`.

# Private API

As a user of Measurement Kit you cannot access the functionality
documented here, because it is not declared in public headers.

However, this documentation could be interesting for Measurement Kit
developers, as it defines internally used interfaces.

Of course, being this API only used internally, do not expect it
to necessarily be stable for a long time.
