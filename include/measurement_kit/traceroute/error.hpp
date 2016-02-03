// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace traceroute {

/// Socket create error
class SocketCreateError : public Error {
  public:
    /// Default constructor
    SocketCreateError() : Error(4000, "unknown_failure 4000") {}
};

/// Setsockopt error
class SetsockoptError : public Error {
  public:
    /// Default constructor
    SetsockoptError() : Error(4001, "unknown_failure 4001") {}
};

/// Probe already pending error
class ProbeAlreadyPendingError : public Error {
  public:
    /// Default constructor
    ProbeAlreadyPendingError() : Error(4002, "unknown_failure 4002") {}
};

/// Payload too long error
class PayloadTooLongError : public Error {
  public:
    /// Default constructor
    PayloadTooLongError() : Error(4003, "unknown_failure 4003") {}
};

/// Storage init error
class StorageInitError : public Error {
  public:
    /// Default constructor
    StorageInitError() : Error(4004, "unknown_failure 4004") {}
};

/// Bind error
class BindError : public Error {
  public:
    /// Default constructor
    BindError() : Error(4005, "unknown_failure 4005") {}
};

/// Event new error
class EventNewError : public Error {
  public:
    /// Default constructor
    EventNewError() : Error(4006, "unknown_failure 4006") {}
};

/// Sendto error
class SendtoError : public Error {
  public:
    /// Default constructor
    SendtoError() : Error(4007, "unknown_failure 4007") {}
};

/// No probe pending error
class NoProbePendingError : public Error {
  public:
    /// Default constructor
    NoProbePendingError() : Error(4008, "unknown_failure 4008") {}
};

/// Clock gettime error
class ClockGettimeError : public Error {
  public:
    /// Default constructor
    ClockGettimeError() : Error(4009, "unknown_failure 4009") {}
};

class EventAddError : public Error {
  public:
    /// Default constructor
    EventAddError() : Error(4010, "unknown_failure 4010") {}
};

class SocketAlreadyClosedError : public Error {
  public:
    /// Default constructor
    SocketAlreadyClosedError() : Error(4011, "unknown_failure 4011") {}
};

} // namespace net
} // namespace mk
#endif
