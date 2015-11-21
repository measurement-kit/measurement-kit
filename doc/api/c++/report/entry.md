# NAME
Entry -- Entry of test report.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit;

report::Entry entry;
report::Entry entry("some input");    // Constructor with specific input

YAML::Node entry.report;              // You can use this directly

YAML::Node node = entry["something"]; // Access 'something'
entry["something"] = "foobar";        // Set 'something' entry

std::string str = entry.str();        // Serialize report entry
```

# DESCRIPTION

The `Entry` object is an entry in a multi-entry report. Since this object
is basically just a wrapper around `YAML::Node`, you can basically use this
object as a dictionary and/or as a list. For example:

```C++
report::Entry entry;

entry["resolver"].push_back("8.8.8.8");
entry["resolver"].push_back("53");

YAML::Node query_entry;
query_entry["query_type"] = "A";
query_entry["query_type"]["answers"][0] = nullptr;
query_entry["query_type"]["answers"][1] = nullptr;
query_entry["rtt"] = 3.14;

entry["queries"].push_back(query_entry);
```

# BUGS

This class uses `yaml-cpp` that in turn is based on boost. The latter
contains lots of headers. Hence to recompile measurement-kit (and also
to create software based on t) you need boost headers at hand.

# HISTORY

The `Entry` class appeared in MeasurementKit 1.0.0.
