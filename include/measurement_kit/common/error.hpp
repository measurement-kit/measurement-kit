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
    Error(int e, std::string ooe, Error c)
        : child(new Error(c)), code(e), ooni_error(ooe) {}
    Error(int e, std::string ooe) : code(e), ooni_error(ooe) {}
    Error() : Error(0, "") {}

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
MK_DEFINE_ERR(1, GenericError, "unknown_failure 1")
MK_DEFINE_ERR(2, NotInitializedError, "unknown_failure 2")
MK_DEFINE_ERR(3, ValueError, "unknown_failure 3")
MK_DEFINE_ERR(4, MockedError, "unknown_failure 4")
MK_DEFINE_ERR(5, JsonParseError, "unknown_failure 5")
MK_DEFINE_ERR(6, JsonKeyError, "unknown_failure 6")

} // namespace mk
#endif
