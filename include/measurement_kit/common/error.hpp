// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <exception>
#include <iosfwd>
#include <string>

namespace mk {

/// An error that occurred
class Error : public std::exception {
  public:
    const char *file = ""; ///< Offending file name
    int lineno = 0;        ///< Offending line number
    const char *func = ""; ///< Offending function

    /// Constructor with error code and OONI error
    Error(int e, std::string ooe) {
        if (e != 0 && ooe == "") {
            ooe = "unknown_failure " + std::to_string(e);
        }
        error_ = e;
        ooni_error_ = ooe;
    }

    Error() : Error(0, "") {}               ///< Default constructor (no error)
    operator int() const { return error_; } ///< Cast to integer

    /// Equality operator
    bool operator==(int n) const { return error_ == n; }

    /// Equality operator
    bool operator==(Error e) const { return error_ == e.error_; }

    /// Unequality operator
    bool operator!=(int n) const { return error_ != n; }

    /// Unequality operator
    bool operator!=(Error e) const { return error_ != e.error_; }

    /// Return error as OONI error
    std::string as_ooni_error() { return ooni_error_; }

  private:
    int error_ = 0;
    std::string ooni_error_;
};

#define MK_DECLARE_ERROR(code, name, ooni_error)                               \
    class name##Error : public Error {                                         \
      public:                                                                  \
        name##Error() : Error(code, ooni_error) {}                             \
    }

#define MK_THROW(classname)                                                    \
    do {                                                                       \
        auto error = classname();                                              \
        error.file = __FILE__;                                                 \
        error.lineno = __LINE__;                                               \
        error.func = __func__;                                                 \
        throw error;                                                           \
    } while (0)

MK_DECLARE_ERROR(0, No, "");
MK_DECLARE_ERROR(1, Generic, "");
MK_DECLARE_ERROR(2, MaybeNotInitialized, "");
MK_DECLARE_ERROR(3, NullPointer, "");
MK_DECLARE_ERROR(4, MallocFailed, "");

MK_DECLARE_ERROR(5, EvutilMakeSocketNonblocking, "");
MK_DECLARE_ERROR(6, EvutilParseSockaddrPort, "");
MK_DECLARE_ERROR(7, EvutilMakeListenSocketReuseable, "");

MK_DECLARE_ERROR(8, EventBaseDispatch, "");
MK_DECLARE_ERROR(9, EventBaseLoop, "");
MK_DECLARE_ERROR(10, EventBaseLoopbreak, "");
MK_DECLARE_ERROR(11, EventBaseOnce, "");

MK_DECLARE_ERROR(12, BuffereventSocketNew, "");
MK_DECLARE_ERROR(13, BuffereventSocketConnect, "");
MK_DECLARE_ERROR(14, BuffereventWrite, "");
MK_DECLARE_ERROR(15, BuffereventWriteBuffer, "");
MK_DECLARE_ERROR(16, BuffereventReadBuffer, "");
MK_DECLARE_ERROR(17, BuffereventEnable, "");
MK_DECLARE_ERROR(18, BuffereventDisable, "");
MK_DECLARE_ERROR(19, BuffereventSetTimeouts, "");
MK_DECLARE_ERROR(20, BuffereventOpensslFilterNew, "");

MK_DECLARE_ERROR(21, EvbufferAdd, "");
MK_DECLARE_ERROR(22, EvbufferAddBuffer, "");
MK_DECLARE_ERROR(23, EvbufferPeek, "");
MK_DECLARE_ERROR(24, EvbufferPeekMismatch, "");
MK_DECLARE_ERROR(25, EvbufferDrain, "");
MK_DECLARE_ERROR(26, EvbufferRemoveBuffer, "");
MK_DECLARE_ERROR(27, EvbufferPullup, "");

MK_DECLARE_ERROR(28, Type, "");

} // namespace mk
#endif
