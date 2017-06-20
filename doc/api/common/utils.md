# NAME
utils &mdash; Utility functions

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
namespace mk {

void timeval_now(timeval *);
double time_now();
void utc_time_now(struct tm *);
ErrorOr<std::string> timestamp(const struct tm *);
timeval *timeval_init(timeval *, double);
Error parse_iso8601_utc(std::string ts, std::tm *tmb);

ErrorOr<std::vector<char>> slurpv(std::string);
ErrorOr<std::string> slurp(std::string);

Error overwrite_file(std::string path, std::string content);

bool startswith(std::string s, std::string p);
bool endswith(std::string s, std::string p);

}
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `timeval_now` function fills the `timeval` structure passed as
argument with the current time. The precision should be up to the microsecond
on most Unix systems.

The `time_now` function returns the current time as double. The double is
obtained converting a `timeval` struct to double and hence should have
precision of the microsecond on most Unix systems.

The `utc_time_now()` function should fill the `tm` structure passed as
argument with the current time in UTC.

The `timestamp()` function should convert the `tm` structure passed as
argument into a string properly formatted according to ISO8601. The return
value is wrapped using `ErrorOr<>` because converting to string may fail.

The `timeval_init()` function initializes the `timeval` structure passed
as argument using the `double` passed as argument. This is often used
to arm timers passed internally to libevent. For this reason, in accordance
with libevent conventions, if the `double` argument is negative, `nullptr`
is returned. Typical safe usage looks like this:

```C++
    timeval tv, *tvp = timeval_init(&tv, delta);
    libevent_api(/* some args... */, tvp);
```

The `parse_iso8601_utc()` function parses the string
`ts` formatted as ISO8601 (e.g. `"2012-01-02T03:04:05Z"`) into a `std::tm`
structure, which MUST NOT be `nullptr`. On success, `NoError()` is returned,
otherwise the code returns `ValueError()`.

The slurpv() and slurp() functions read the content of the file specified
as argument and return, respectively, a vector of chars containing the content
of the file and a string containing the content of the file, on success, and
an error in case of failure.

The `overwrite_file()` function creates (or truncates) the file at `path`
and writes `content` inside it. The return value is `NoError()` on success
or an error on failure.

The `startswith()` and `endswith()` functions tell you, respectively, whether
`s` starts (or ends) with `p`.

# HISTORY

The `utils.hpp` header was added in MeasurementKit 0.2.0.
