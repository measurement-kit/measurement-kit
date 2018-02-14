// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_PLATFORM_HPP
#define MEASUREMENT_KIT_COMMON_PLATFORM_HPP

#ifdef __cplusplus
extern "C" {
#endif

/// \brief `mk_platform` returns the operating system platform.
///
/// \return `"android"` on Android.
///
/// \return `"linux"` on Linux.
///
/// \return `"windows"` on Windows.
///
/// \return `"ios"` on iOS.
///
/// \return `"macos"` on macOS.
///
/// \return `"unknown"` otherwise.
const char *mk_platform(void);

#ifdef __cplusplus
} // namespace mk
#endif
#endif
