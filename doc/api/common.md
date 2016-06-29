# NAME
common &mdash; common functionality using by other modules

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>
```

# DESCRIPTION

The `common` module contains common functionality in MeasurementKit
that can all be pulled by including the `common.hpp` header.

This module contains the following submodules:

- [callback](common/callback.md): alias for callback functions
- [continuation](common/continuation.md): function to restart a paused function (i.e. a simple coroutine)
- [delegate](common/delegate.md): overridable member function
- [error](common/error.md): represents an error
- [error_or](common/error_or.md): contains a value or an error
- [lexical_cast](common/lexical_cast.md): cast between scalar values
- [logger](common/logger.md): functionality to log messages
- [mock](common/mock.md): macros to mock implementation and write tests
- [net_test](common/net_test.md): generic network test
- [non_copyable](common/non_copyable.md): non copyable class
- [non_movable](common/non_movable.md): non movable class
- [reactor](common/reactor.md): class for dispatching I/O events and timeouts
- [runner](common/runner.md): class for running network tests and manage theif lifecycle
- [settings](common/settings.md): class containing test settings
- [settings_entry](common/settings_entry.md): a setting inside test settings
- [utils](common/utils.md): generic utility functions
- [var](common/var.md): shared smart pointer with null pointer checks
- [version](common/version.md): MeasurementKit version macro

# HISTORY

The `common` library appeared in MeasurementKit 0.1.0.
