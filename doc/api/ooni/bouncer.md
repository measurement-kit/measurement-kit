# NAME
bouncer -- Routines to interact with OONI's bouncer

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

class BouncerReply {
  public:
    nlohmann::json response;
    static ErrorOr<Var<BouncerReply>> create(std::string, Var<Logger>);
    ErrorOr<std::string> get_collector();
    ErrorOr<std::string> get_collector_alternate(std::string type);
    ErrorOr<std::string> get_name();
    ErrorOr<std::string> get_test_helper(std::string name);
    ErrorOr<std::string> get_test_helper_alternate(std::string name,
                                                   std::string type);
    ErrorOr<std::string> get_version();
  private:
    nlohmann::json get_base();
};

void mk::ooni::bouncer::post_net_tests(std::string base_bouncer_url, 
                    std::string test_name, std::string test_version,
                    std::list<std::string> helpers, 
		    Callback<Error, Var<BouncerReply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger);
```

# STABILITY

1 - Experimental

# DESCRIPTION

This header contains routines to interact with OONI's bouncer.

The `post_net_tests` function allows to query the OONI's bouncer. The 
first argument is the `bouncer_url`. The second argument is the name of
the test for which you need a collector. The third argument is the version
of the test. The fourth argument is a list of strings containing the name of
the test helpers you need. The fifth argument is a callback function called
when done, with an Error or `NoError()` in case of success. In that case
it will receive also a `BouncerReply` object.

The `BouncerReply` class can be used to interact with an OONI's bouncer
response without using directly the received JSON.

The `BouncerReply` class has getter methods to access the values of
the Bouncer response. The names of the methods are self explanatory.
They don't throw exceptions.

A new instance of this class can be created with the static constructor
method `create()` giving a valid JSON and a Logger to it. This static
method doesn't throw exceptions and returns an `ErrorOr` that can contain 
an `Error` of type:

```
- BouncerCollectorNotFoundError()
- BouncerTestHelperNotFoundError()
- BouncerInvalidRequestError()
- BouncerGenericError()
- BouncerValueNotFoundError()
```

# HISTORY

The `bouncer` module appeared in MeasurementKit 0.4.0.
