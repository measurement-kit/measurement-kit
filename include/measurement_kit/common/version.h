// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_VERSION_HPP
#define MEASUREMENT_KIT_COMMON_VERSION_HPP

// Note: we use semantic versioning (see: http://semver.org/)
#define MEASUREMENT_KIT_VERSION "0.5.0-alpha"

#ifdef __cplusplus
extern "C" {
#endif

const char *mk_version(void);

#ifdef __cplusplus
} // namespace mk
#endif
#endif
