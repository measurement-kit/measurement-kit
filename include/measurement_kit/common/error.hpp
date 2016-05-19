// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <exception>
#include <iosfwd>
#include <measurement_kit/common/var.hpp>
#include <string>

namespace mk {

class ErrorContext {};

class Error : public std::exception {
  public:
    Error(int e, std::string ooe, Var<Error> c)
                : child(c), code(e), ooni_error(ooe) {
        if (code != 0 && ooni_error == "") {
            ooni_error = "unknown_failure " + std::to_string(code);
        }
    }

    Error(int e, std::string ooe, Error c)
        : Error(e, ooe, Var<Error>(new Error(c))) {}
    Error(int e, std::string ooe) : Error(e, ooe, nullptr) {}
    Error(int e) : Error(e, "", nullptr) {}
    Error() : Error(0, "", nullptr) {}

    operator bool() const { return code != 0; }

    bool operator==(int n) const { return code == n; }
    bool operator==(Error e) const { return code == e.code; }
    bool operator!=(int n) const { return code != n; }
    bool operator!=(Error e) const { return code != e.code; }

    std::string as_ooni_error() { return ooni_error; }

    Var<ErrorContext> context;
    Var<Error> child;
    int code = 0;
    std::string ooni_error;
};

#define MK_DEFINE_ERR(_code_, _name_, _ooe_)                                   \
    class _name_ : public Error {                                              \
      public:                                                                  \
        _name_() : Error(_code_, _ooe_) {}                                     \
        _name_(Error e) : Error(_code_, _ooe_, e) {}                           \
    };

MK_DEFINE_ERR(0, NoError, "")
MK_DEFINE_ERR(1, GenericError, "")
MK_DEFINE_ERR(2, NotInitializedError, "")
MK_DEFINE_ERR(3, ValueError, "")
MK_DEFINE_ERR(4, MockedError, "")
MK_DEFINE_ERR(5, JsonParseError, "")
MK_DEFINE_ERR(6, JsonKeyError, "")

} // namespace mk
#endif
