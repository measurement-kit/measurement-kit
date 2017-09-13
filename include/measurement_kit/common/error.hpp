// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <string>
#include <vector>

namespace mk {

class Error : public std::exception {
  public:
    Error() : Error(0, "") {}

    Error(int e) : Error(e, "") {}

    Error(int e, std::string r) : code{e}, reason{r} {
        if (code != 0 && reason == "") {
            reason = "unknown_failure " + std::to_string(code);
        }
    }

    Error(int e, std::string r, const Error &c) : Error(e, r) {
        child_errors.push_back(c);
    }

    operator bool() const { return code != 0; }

    bool operator==(int n) const { return code == n; }

    bool operator==(Error e) const { return code == e.code; }

    bool operator!=(int n) const { return code != n; }

    bool operator!=(Error e) const { return code != e.code; }

    const char *what() const noexcept override { return reason.c_str(); }

    void add_child_error(const Error &err) {
        child_errors.push_back(err);
    }

    std::vector<Error> child_errors;
    int code = 0;
    std::string reason;
};

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

MK_DEFINE_ERR(0, NoError, "")
MK_DEFINE_ERR(1, GenericError, "generic_error")
MK_DEFINE_ERR(2, NotInitializedError, "not_initialized")
MK_DEFINE_ERR(3, ValueError, "value_error")
MK_DEFINE_ERR(4, MockedError, "mocked_error")
MK_DEFINE_ERR(5, JsonParseError, "json_parse_error")
MK_DEFINE_ERR(6, JsonKeyError, "json_key_error")
MK_DEFINE_ERR(7, JsonDomainError, "json_domain_error")
MK_DEFINE_ERR(8, FileEofError, "file_eof_error")
MK_DEFINE_ERR(9, FileIoError, "file_io_error")
MK_DEFINE_ERR(10, ParallelOperationError, "parallel_operation_error")
MK_DEFINE_ERR(11, SequentialOperationError, "sequential_operation_error")
MK_DEFINE_ERR(12, IllegalSequenceError, "illegal_sequence")
MK_DEFINE_ERR(13, UnexpectedNullByteError, "unexpected_null_byte")
MK_DEFINE_ERR(14, IncompleteUtf8SequenceError, "incomplete_utf8_sequence")
MK_DEFINE_ERR(15, NotImplementedError, "not_implemented")
MK_DEFINE_ERR(16, TimeoutError, "generic_timeout_error")

#define MK_ERR_NET(x) (1000 + x)
#define MK_ERR_DNS(x) (2000 + x)
#define MK_ERR_HTTP(x) (3000 + x)
#define MK_ERR_TRACEROUTE(x) (4000 + x)
#define MK_ERR_MLABNS(x) (5000 + x)
#define MK_ERR_OONI(x) (6000 + x)
#define MK_ERR_REPORT(x) (7000 + x)
#define MK_ERR_NDT(x) (8000 + x)

} // namespace mk

namespace std {

inline std::ostream &operator<<(std::ostream &os, const mk::Error &value) {
    os << value.reason;
    return os;
}

} // namespace std
#endif
