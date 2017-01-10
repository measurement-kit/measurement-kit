# NAME
encoding &mdash; Handle encodings (e.g. UTF-8, base64)

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

Error is_valid_utf8_string(const std::string &s);

std::string base64_encode(std::string s);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `is_valid_utf8_string` function returns `NoError()` if the string `s`
contains a *valid and complete* UTF-8 string. Otherwise, it SHOULD return
one of the following errors:

- `IllegalSequenceError`: if the string contains an invalid UTF-8 sequence
- `UnexpectedNullByteError`: if there is a `null` byte in the middle
- `IncompleteUtf8SequenceError`: if the string does not end with a complete
  multibyte character serialized as UTF-8

Internally, `is_valid_utf8_string` MAY be implemented using `mbrtowc(3)`
configured for processing UTF-8. `mbrtowc(3)` is part of ISO C 90.

The `base64_encode` function encodes the string passed in input as base64.

# HISTORY

The `is_complete_utf8_string` and `base64_encode` functions appeared in
MeasurementKit 0.4.0.
