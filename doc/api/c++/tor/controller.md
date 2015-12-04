# NAME
Controller -- Tor controller.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/tor.hpp>

using namespace mk::tor;

// Default constructor using 127.0.0.1:9050
Controller controller([]() {
    // Here you must actually start Tor using CPAProxy
});

// Constructor with custom address and port
Controller controller("127.0.0.1", 9051, []() {
    // Here you must actually start Tor using CPAProxy
});

controller.on_complete([](common::Error error) {
    // Check error to see whether Tor started successfully or not
});

controller.on_progress([](int code, std::string reason) {
    // Provide information on Tor startup process
});

controller.start_tor();
```

# DESCRIPTION

The `Controller` class provides a common interface for running Tor
in different contexts (iOS and Android).

Basically you need to create and instance of `Controller` specifying
at least the function that would actually start Tor.

On iOS to acually start Tor we use CPAProxy, a library that implements
a Tor controller in ObjectiveC.

# BUGS

We have not implemented yet a mechanism to stop Tor. This mechanism is
needed because we don't want to keep the Tor connection open for the whole
lifecycle of a measurement-kit App.

# HISTORY

The `Contoller` class appeared in MeasurementKit 0.1.
