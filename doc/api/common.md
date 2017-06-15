# NAME
common &mdash; common functionality used by other modules

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>
```

# DESCRIPTION

The `common` module contains contains the following submodules:

- [aaa_base](common/aaa_base.md): include headers we always need
- [callback](common/callback.md): alias for writing callback functions more compactly
- [continuation](common/continuation.md): type of function used to restart a function that paused
- [delegate](common/delegate.md): member function overridable at runtime
- [encoding](common/encoding.md): handle encodings (e.g. UTF-8, base64)
- [error](common/error.md): definition of error class and of shared error codes
- [error_or](common/error_or.md): maybe-like object containing either a result or an error
- [fmap](common/fmap.md): apply a function to a vector
- [lexical_cast](common/lexical_cast.md): lexical cast between scalar values
- [logger](common/logger.md): log messages
- [mock](common/mock.md): macros to mock implementation and write tests
- [non_copyable](common/non_copyable.md): non copyable class
- [non_movable](common/non_movable.md): non movable class
- [parallel](common/parallel.md): allows to run continuations in parallel
- [range](common/range.md): generates numbers from zero to N
- [reactor](common/reactor.md): class for dispatching I/O events and timeouts
- [settings](common/settings.md): class containing test settings
- [settings_entry](common/settings_entry.md): an entry contained by the settings class
- [utils](common/utils.md): generic utility functions
- [var](common/var.md): shared smart pointer with null pointer checks
- [version](common/version.md): MeasurementKit version macro

# HISTORY

The `common` module appeared in MeasurementKit 0.1.0.
