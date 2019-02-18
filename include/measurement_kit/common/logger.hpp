// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LOGGER_HPP
#define MEASUREMENT_KIT_COMMON_LOGGER_HPP

// The numbers [0-31] are reserved for verbosity levels.

/// `MK_LOG_QUIET` indicates that no log should be emitted.
#define MK_LOG_QUIET 0

/// `MK_LOG_ERR` indicates the `ERR` log severity level.
#define MK_LOG_ERR 1

/// `MK_LOG_WARNING` indicates the `WARNING` log severity level.
#define MK_LOG_WARNING 2

/// `MK_LOG_INFO` indicates the `INFO` log severity level.
#define MK_LOG_INFO 3

/// `MK_LOG_DEBUG` indicates the `DEBUG` log severity level.
#define MK_LOG_DEBUG 4

/// `MK_LOG_DEBUG2` indicates the `DEBUG2` log severity level.
#define MK_LOG_DEBUG2 5

/// \brief `MK_LOG_VERBOSITY_MASK` is a bitmask indicating which bits
/// are being used to specify the severity level. Bits above such mask
/// have another semantic.
#define MK_LOG_VERBOSITY_MASK 31

// The number above 31 have different semantics:

/// \brief `MK_LOG_EVENT` indicates an event. It is a bit outside of the
/// verbosity mask. This is used to indicate that the current log message
/// is not plaintext but rather a serialized JSON representing an event.
#define MK_LOG_EVENT 32

#endif  // MEASUREMENT_KIT_COMMON_LOGGER_HPP
