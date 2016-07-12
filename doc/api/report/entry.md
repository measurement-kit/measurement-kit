# NAME
entry &mdash; Report entry

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

class Entry : private nlohmann::json {
  public:
    using nlohmann::json::json;

    static Entry array();
    template <typename T> operator ErrorOr<T>();

    Entry &operator=(Entry value);
    Entry &operator[](std::string key);

    void push_back(Entry);

    std::string dump();

    bool operator==(std::nullptr_t right);
    bool operator!=(std::nullptr_t right);
};

} // namespace report
} // namespace mk
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `Entry` class inherits privately from `nlohmann::json` (pulled from `ext`) and behaves
like a JSON object (i.e. an object with both dictionary and list semantics, depending on
the context).

# BUGS

Currently the `Entry` object is quite difficult to manipulate; we should perhaps just
implement it as an alias for `nlohmman::json` to simplify its use (as opposed to creating
a wrapper for each method of such base class to which we are interested).

# HISTORY

The `entry` module appeared in MeasurementKit 0.2.0.
