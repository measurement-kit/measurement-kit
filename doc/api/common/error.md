# NAME
Error &mdash; Represent and handle errors.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class ErrorContext {};

class Error : public std::exception {
  public:
    Error();
    Error(int code);
    Error(int code, std::string reason);
    Error(int code, std::string reason, Error child);

    operator bool() const;

    bool operator==(int code);
    bool operator==(Error error);
    bool operator!=(int code);
    bool operator!=(Error error);

    std::string as_ooni_error();

    Var<ErrorContext> context;
    Var<Error> child;
    int code = 0;
    std::string reason;
};

#define MK_DEFINE_ERR(code_, ClassName_, reason_)           \
    class ClassName_ : public Error {                       \
      public:                                               \
        ClassName_() : Error(code_, reason_) {}             \
        ClassName_(std::string s) : Error(code_, reason_) { \
            this->reason += " ";                            \
            this->reason += s;                              \
        }                                                   \
    };

MK_DEFINE_ERR(0, NoError, "")
MK_DEFINE_ERR(1, GenericError, "")
MK_DEFINE_ERR(2, NotInitializedError, "")
MK_DEFINE_ERR(3, ValueError, "")
MK_DEFINE_ERR(4, MockedError, "")
MK_DEFINE_ERR(5, JsonParseError, "")
MK_DEFINE_ERR(6, JsonKeyError, "")
MK_DEFINE_ERR(7, JsonDomainError, "")
MK_DEFINE_ERR(8, FileEofError, "")
MK_DEFINE_ERR(9, FileIoError, "")

#define MK_ERR_NET(x) (1000 + x)
#define MK_ERR_DNS(x) (2000 + x)
#define MK_ERR_HTTP(x) (3000 + x)
#define MK_ERR_TRACEROUTE(x) (4000 + x)
#define MK_ERR_MLABNS(x) (5000 + x)
#define MK_ERR_OONI(x) (6000 + x)
#define MK_ERR_REPORT(x) (7000 + x)
#define MK_ERR_NDT(x) (8000 + x)

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `ErrorContext` class is a generic class that can be used as a base class
of any type of error context that could be stored within an error.

The `Error` class represents an error. It is a derived class of `std::exception`
so that it could be thrown and catched as an exception. (In general in MeasurementKit
we prefer passing errors to callbacks rather than throwing them as exceptions, but
there are some cases where throwing makes sense.)

The `Error` class has four constructors. The empty constructor constructs an error
with error code equal to zero (a special value used to indicate that no error has
actually occurred). The constructor with error code allows to set the error code that
is wrapped by an error; a nonzero error code indicates failure. The constructor
with error code and reason allows to set both error code and a reason string; at the
moment the reason string MUST be a [OONI error string](https://github.com/TheTorProject/ooni-spec/blob/master/data-formats/df-000-base.md#error-strings),
but this MAY change in the future. The constructor with error code, reason, and
underlying error allows also to specify that the current error was triggered by
another, underlying error.

The bool operator returns true if the `Error` code is nonzero and false if instead
the error code is zero. It can be used to quickly check whether an error occurred
as in the following snippet of code:

```C++
    func([=](Error err, Result result) {
        if (err) {
            callback(err);
            return;
        }
        process_result(result);
    });
```

The equality and inequality operators allow to compare errors with error codes and
with errors. Currently, the comparison is *only* performed on the grounds of the
error code, hence different error classes having the same error code would be equal,
as shown in the following snippet of code:

```C++
    MK_DEFINE_ERR(7, FooError, "");
    MK_DEFINE_ERR(7, FoobarError, "");
    REQUIRE((FooError() == FoobarError()));
```

The `as_ooni_error()` method allows to obtain the [OONI error string](https://github.com/TheTorProject/ooni-spec/blob/master/data-formats/df-000-base.md#error-strings) corresponding
to a specific MeasurementKit error. In the future the error returned by `as_ooni_error()`
MAY be different from the error stored in the `reason` field.

The `Var<ErrorContext> context` field is a shared smart pointer (see `Var`) that MAY
store the error context in specific cases. The typical pattern for accessing such error
context involves three steps: making sure that the context is not `nullptr`, casting
the base error class to the specific expected class, and making sure that the cast did
not fail, again checking whether the context is not `nullptr`. For example:

```C++
    operation([=](Error err) {
        if (err) {
            if (err.context) {
                Var<SpecificContext> ctx = err.context.as<SpecificContext>();
                if (ctx) {
                    // TODO: use the specific context
                }
            }
            return;
        }
        // Normal processing...
    });
```

The `Var<Error> child` field is a smart pointer that MAY be set to indicate
that the current error was provoked by another underlying error. This should
allow the programmer to properly layer errors. Thus, one could learn that, say, a
specific call to an API failed, and that the reason why that failed was that
it was not possible to establish a network connection, and that the reason
why that was not possible is that the DNS failed.

The `code` and `reason` fields are, respectively, the integer error code
uniquely identifying the error and the related textual description. The latter
MAY NOT necessarily be equal to the value returned by `as_ooni_error()`.

The `MK_DEFINE_ERR` macro allows to compactly define a derived class Error by
specifying the error code, the class name, and the reason. In addition to
the vanilla error, a derived error has one extra constructor allowing to pass
additional textual context to be added to the reason.

The `measurement_kit/common.hpp` header uses `MK_DEFINE_ERR` to define *at least*
also the following generic error codes:

- *NoError*: this class MUST have error code equal to zero and SHOULD be used
  to indicate that no error occurred

- *GenericError*: this class MAY be used to indicate a generic error but it
  SHOULD NOT be widely used and specific errors SHOULD be preferred

- *NotInitializedError*: this is the error that SHOULD be used when accessing
  a contained that requires initialization and was not primed

- *ValueError*: this error SHOULD be returned when it is not possible to convert
  a specific value from one form to another (e.g. a string to integer)

- *MockedError*: this error SHOULD be used in regress tests when mocking the
  failure of an API, unless specific other errors are required

- *JsonParseError*: indicates that it was not possible to parse a JSON

- *JsonKeyError*: indicates that a JSON object does not contain a specific key

To make sure that errors belonging to different MeasurementKit libraries are
disjoint, `measurement_kit/common.hpp` also defines offsets at which errors of each library
SHOULD start, as well as useful macros that takes codes relative to a specific
library and yields absolute error codes. Each MeasurementKit library is given
room for up to 1,000 error codes. At least the following offsets SHOULD be
defined by the `measurement_kit/common.hpp` header:

- 0 for the *common* lib
- 1,000 for the *net* lib
- 2,000 for the *dns* lib
- 3,000 for the *http* lib
- 4,000 for the *traceroute* lib
- 5,000 for the *mlabns* lib
- 6,000 for the *ooni* lib
- 7,000 for the *report* lib
- 8,000 for the *ndt* lib

# BUGS

Since the only integral value to which `Error` could be converted is `bool` there
are odd cases where the following statement:

```C++
    REQUIRE(err == SomeError());
```

may fail because the two errors are different but `Catch` may still print as
reason for the failed error `1 == 1` because both classes evaluate to `true` (i.e. `1`)
even though they internally contain two different errors.

# HISTORY

The `Error` class appeared in MeasurementKit 0.1.0. The `ErrorContext` class, the
`MK_DEFINE_ERR` macro, and the macros to compute absolute error codes all appeared
in MeasurementKit 0.2.0.
