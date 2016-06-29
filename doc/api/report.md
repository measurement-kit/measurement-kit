# NAME
report &mdash; Results of a test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>
```

# DESCRIPTION

In OONI, a test consists of one or more test cases. Each test case produces as
result an entry. All such entries make a report.

The `report` module contains the following submodules:

- [entry](report/entry.md): contains the results of a single test case
- [file reporter](report/file_reporter.md): writes entries on disk

# HISTORY

The `report` module appeared in MeasurementKit 0.2.0.
