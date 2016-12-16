// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <measurement_kit/common/var.hpp>

#include <string>
#include <vector>

namespace mk {

class ErrorContext {};

class Error : public std::exception {
  public:
    Error() : Error(0, "", nullptr) {}
    Error(int e) : Error(e, "", nullptr) {}
    Error(int e, std::string ooe) : Error(e, ooe, nullptr) {}

    Error(int e, std::string ooe, Var<Error> c) : code(e), reason(ooe) {
        if (code != 0 && reason == "") {
            reason = "unknown_failure " + std::to_string(code);
        }
        if (c) {
            child_errors.push_back(c);
        }
    }

    Error(int e, std::string ooe, Error c)
        : Error(e, ooe, Var<Error>(new Error(c))) {}

    operator bool() const { return code != 0; }

    bool operator==(int n) const { return code == n; }
    bool operator==(Error e) const { return code == e.code; }
    bool operator!=(int n) const { return code != n; }
    bool operator!=(Error e) const { return code != e.code; }

    std::string as_ooni_error() { return reason; }

    void add_child_error(const Error &err) {
        Var<Error> container(new Error(err));
        child_errors.push_back(container);
    }

    std::string explain() const {
        std::string s;
        s += "{";
        s += reason;
        s += "}";
        if (child_errors.size() > 0) {
            s += " [";
            for (auto &e : child_errors) {
                s += e->explain();
            }
            s += "]";
        }
        return s;
    }

    Var<ErrorContext> context;
    std::vector<Var<Error>> child_errors;
    int code = 0;
    std::string reason;
};

#define MK_DEFINE_ERR(_code_, _name_, _ooe_)                                   \
    class _name_ : public Error {                                              \
      public:                                                                  \
        _name_() : Error(_code_, _ooe_) {}                                     \
        _name_(std::string s) : Error(_code_, _ooe_) {                         \
            reason += " ";                                                     \
            reason += s;                                                       \
        }                                                                      \
        _name_(Error e) : Error(_code_, _ooe_, e) {}                           \
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
MK_DEFINE_ERR(10, ParallelOperationError, "")
MK_DEFINE_ERR(11, SequentialOperationError, "")
MK_DEFINE_ERR(12, UncaughtExceptionError, "")

#define MK_ERR_NET(x) (1000 + x)
#define MK_ERR_DNS(x) (2000 + x)
#define MK_ERR_HTTP(x) (3000 + x)
#define MK_ERR_TRACEROUTE(x) (4000 + x)
#define MK_ERR_MLABNS(x) (5000 + x)
#define MK_ERR_OONI(x) (6000 + x)
#define MK_ERR_REPORT(x) (7000 + x)
#define MK_ERR_NDT(x) (8000 + x)

} // namespace mk
#endif
