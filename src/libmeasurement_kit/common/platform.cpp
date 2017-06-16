// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

/*
 * Guess the platform in which we are.
 *
 * See: <https://sourceforge.net/p/predef/wiki/OperatingSystems/>
 *      <http://stackoverflow.com/a/18729350>
 */
#if defined __ANDROID__
#  define MK_PLATFORM "android"
#elif defined __linux__
#  define MK_PLATFORM "linux"
#elif defined _WIN32
#  define MK_PLATFORM "windows"
#elif defined __APPLE__
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
#    define MK_PLATFORM "ios"
#  else
#    define MK_PLATFORM "macos"
#  endif
#else
#  define MK_PLATFORM "unknown"
#endif

const char *mk_platform() { return MK_PLATFORM; }
