// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <string>
#include <vector>

namespace mk {

/// \brief `Error` represents an error that occurred. It is a derived class of
/// `std::exception` such that one can throw `Errors` and more generally catch
/// all `std::exception`. (In general in measurement-kit we would rather pass
/// error to callbacks than throw, but there are also some cases where throwing
/// makes sense.)
///
/// An error is uniquely identified by the error code that it wraps. By
/// popular convention, the `0` error code indicates no error.
///
/// In addition to the error code, there is also a reason string that
/// indicates the error that occurred in a more human friendly way. The
/// reason strings are compatible with the failure strings [specified
/// by
/// OONI](https://github.com/TheTorProject/ooni-spec/blob/master/data-formats/df-000-base.md#error-strings).
///
/// Error codes may change from one version of measurement-kit to the
/// following one. Reason strings instead should not change unless that
/// is required to track OONI's specification.
///
/// The reason string corresponding to no error is an empty string.
///
/// If there is no reason string corresponding to a specific error code, the
/// default reason string `unknown_error ${code}` is generated.
///
/// An error can contain child errors. This is handy when representing the
/// result of a parallel operation where only certain sub-operations may fail.
///
/// As of measurement-kit v0.8.0, too many errors are defined. We will try
/// to cut down the total number of errors to around 50.
///
/// Error appeared in measurement-kit v0.1.0. `MK_DEFINE_ERR` was added
/// in measurement-kit v0.2.0. Between v0.2.0 and v0.7.0 (excluded) we
/// also had the `ErrorContext` class. The Error has been further reworked
/// as part of measurement-kit v0.8.0 as part of a refactoring of common.
class Error : public std::exception {
  public:

    /// \brief The default constructor initializes the error code to zero.
    Error() : Error(0, "") {}

    /// \brief The constructor with error initializes the error code to the
    /// specified error code.
    Error(int e) : Error(e, "") {}

    /// \brief The constructor with error and reason string initializes both.
    Error(int e, std::string r) : code{e}, reason{r} {
        if (code != 0 && reason == "") {
            reason = "unknown_failure " + std::to_string(code);
        }
    }

    /// \brief The constructor with error code, reason string, and child
    /// error also initializes the child error.
    Error(int e, std::string r, const Error &c) : Error(e, r) {
        child_errors.push_back(c);
    }

    /// The bool operator evaluates to true if the error code is nonzero.
    operator bool() const { return code != 0; }

    /// \brief The equality with error code operator compares the error's
    /// error code with the specified error code.
    bool operator==(int n) const { return code == n; }

    /// The equality with error code compares the error codes for equality.
    bool operator==(Error e) const { return code == e.code; }

    /// \brief The unequality with error code operator compares the error's
    /// error code with the specified error code.
    bool operator!=(int n) const { return code != n; }

    /// The unequality with error compares the error codes for unquality.
    bool operator!=(Error e) const { return code != e.code; }

    /// `what` returns the reason string.
    const char *what() const noexcept override { return reason.c_str(); }

    /// `add_child_error` allows you tou append additional child errors.
    void add_child_error(const Error &err) {
        child_errors.push_back(err);
    }

    /// `child_errors` contains all the child errors.
    std::vector<Error> child_errors;

    /// `code` contains the error code.
    int code = 0;

    /// `reason` contains the reason string.
    std::string reason;
};

/// `MK_DEFINE_ERR` allows you to quickly define a class derived from Error.
#define MK_DEFINE_ERR(_code_, _name_, _reason_)                                \
    class _name_ : public Error {                                              \
      public:                                                                  \
        _name_() : Error(_code_, _reason_) {}                                  \
                                                                               \
        _name_(std::string s) : Error(_code_, _reason_) {                      \
            reason += ": ";                                                    \
            reason += s;                                                       \
        }                                                                      \
                                                                               \
        _name_(Error e) : Error(_code_, _reason_, e) {}                        \
    };

/// `NoError` represents the absence of errors.
MK_DEFINE_ERR(0, NoError, "")

/// \brief `GenericError` represents all the errors that we don't bother
/// to map more specifically.
MK_DEFINE_ERR(1, GenericError, "generic_error")

/// \brief `NotInitializedError` is used when a variable that should have
/// been initialized was not.
MK_DEFINE_ERR(2, NotInitializedError, "not_initialized")

/// `ValueError` is used whenever the provided input is invalid.
MK_DEFINE_ERR(3, ValueError, "value_error")

/// \brief `MockedError` should only be used in regress tests to indicate
/// that a specific API failed with a mocked error. This is used to make
/// sure that the returned error was actually correctly mocked.
MK_DEFINE_ERR(4, MockedError, "mocked_error")

/// `JsonParseError` indicates that we could not parse a JSON.
MK_DEFINE_ERR(5, JsonParseError, "json_parse_error")

/// `JsonKeyError` indicates that a specific key does not exists in a JSON.
MK_DEFINE_ERR(6, JsonKeyError, "json_key_error")

/// \brief `JsonDomainError` indicates that we are using a JSON of
/// a specific type assuming that it is of another type. For example, the
/// JSON may be `null` and we may be trying to using it like a map from
/// string to string, which obviously is not possible.
MK_DEFINE_ERR(7, JsonDomainError, "json_domain_error")

/// `FileEofError` indicates that we hit premature EOF when reading a file.
MK_DEFINE_ERR(8, FileEofError, "file_eof_error")

/// `FileIoError` indicates I/O error when reading a file.
MK_DEFINE_ERR(9, FileIoError, "file_io_error")

/// \brief `ParallelOperationError` indicates that a parallel operation failed.
/// In this case, consult the child errors to see more details.
MK_DEFINE_ERR(10, ParallelOperationError, "parallel_operation_error")

/// \brief `SequentialOperationError` indicates that a sequential operation
/// has failed. See the child errors for more details.
MK_DEFINE_ERR(11, SequentialOperationError, "sequential_operation_error")

/// `IllegalSequenceError` indicates an illegal UTF-8 sequence.
MK_DEFINE_ERR(12, IllegalSequenceError, "illegal_sequence")

/// \brief `UnexpectedNullByteError` indicates an unexpected null byte in a
/// UTF-8 stream.
MK_DEFINE_ERR(13, UnexpectedNullByteError, "unexpected_null_byte")

/// `IncompleteUtf8SequenceError` indicates an unexpected UTF-8 sequence.
MK_DEFINE_ERR(14, IncompleteUtf8SequenceError, "incomplete_utf8_sequence")

/// `NotImplementedError` indicates that an operation is not implemented.
MK_DEFINE_ERR(15, NotImplementedError, "not_implemented")

/// `TimeoutError` indicates that a timeout occurred.
MK_DEFINE_ERR(16, TimeoutError, "generic_timeout_error")

/// `JsonProcessingError` indicates an error processing a JSON.
MK_DEFINE_ERR(17, JsonProcessingError, "json_processing_error")

/// \brief `MK_ERR_NET` takes a relative error code and returns an error code
/// inside of the error codes space reserved for the net sub-library.
#define MK_ERR_NET(x) (1000 + x)

/// `MK_ERR_DNS` is like MK_ERR_NET but for dns.
#define MK_ERR_DNS(x) (2000 + x)

/// `MK_ERR_HTTP` is like MK_ERR_NET but for http.
#define MK_ERR_HTTP(x) (3000 + x)

/// `MK_ERR_TRACEROUTE` is like MK_ERR_NET but for traceroute.
#define MK_ERR_TRACEROUTE(x) (4000 + x)

/// `MK_ERR_MLABNS` is like MK_ERR_NET but for mlabns.
#define MK_ERR_MLABNS(x) (5000 + x)

/// `MK_ERR_OONI` is like MK_ERR_NET but for ooni.
#define MK_ERR_OONI(x) (6000 + x)

/// `MK_ERR_REPORT` is like MK_ERR_NET but for report.
#define MK_ERR_REPORT(x) (7000 + x)

/// `MK_ERR_NDT` is like MK_ERR_NET but for ndt.
#define MK_ERR_NDT(x) (8000 + x)

/// `MK_ERR_NEUBOT` is like MK_ERR_NET but for neubot.
#define MK_ERR_NEUBOT(x) (9000 + x)

} // namespace mk

namespace std {

/// `std::operator<<`'s override allows to print errors as their reason string.
inline std::ostream &operator<<(std::ostream &os, const mk::Error &value) {
    os << value.reason;
    return os;
}

} // namespace std
#endif
