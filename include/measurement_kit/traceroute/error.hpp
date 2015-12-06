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
    SocketCreateError() : Error(2000, "unknown_failure 2000") {}
};

/// Setsockopt error
class SetsockoptError : public Error {
  public:
    /// Default constructor
    SetsockoptError() : Error(2001, "unknown_failure 2001") {}
};

/// Probe already pending error
class ProbeAlreadyPendingError : public Error {
  public:
    /// Default constructor
    ProbeAlreadyPendingError() : Error(2002, "unknown_failure 2002") {}
};

/// Payload too long error
class PayloadTooLongError : public Error {
  public:
    /// Default constructor
    PayloadTooLongError() : Error(2003, "unknown_failure 2003") {}
};

/// Storage init error
class StorageInitError : public Error {
  public:
    /// Default constructor
    StorageInitError() : Error(2004, "unknown_failure 2004") {}
};

/// Bind error
class BindError : public Error {
  public:
    /// Default constructor
    BindError() : Error(2005, "unknown_failure 2005") {}
};

/// Event new error
class EventNewError : public Error {
  public:
    /// Default constructor
    EventNewError() : Error(2006, "unknown_failure 2006") {}
};

/// Sendto error
class SendtoError : public Error {
  public:
    /// Default constructor
    SendtoError() : Error(2007, "unknown_failure 2007") {}
};

/// No probe pending error
class NoProbePendingError : public Error {
  public:
    /// Default constructor
    NoProbePendingError() : Error(2008, "unknown_failure 2008") {}
};

/// Clock gettime error
class ClockGettimeError : public Error {
  public:
    /// Default constructor
    ClockGettimeError() : Error(2009, "unknown_failure 2009") {}
};

class EventAddError : public Error {
  public:
    /// Default constructor
    EventAddError() : Error(2010, "unknown_failure 2010") {}
};

class SocketAlreadyClosedError : public Error {
  public:
    /// Default constructor
    SocketAlreadyClosedError() : Error(2011, "unknown_failure 2011") {}
};

} // namespace net
} // namespace mk
#endif
