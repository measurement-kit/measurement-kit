// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_API_H
#define MEASUREMENT_KIT_COMMON_API_H

// clang-format off

#ifdef DOXYGEN

/// MK_API is a macro that allows you to control the visibility of
/// symbols when building dynamic libraries. By default, it expands to nothing,
/// which is suitable for building static libraries. You should define
/// MK_BUILD_DLL when you are creating the dynamic library. Instead,
/// when the objective is to link with a dynamic libary, make sure that
/// MK_USE_DLL is defined.
#  define MK_API /* System and build flags dependent. */

#elif defined MK_BUILD_DLL

#  if defined __GNUC__ && __GNUC__ >= 4
#    define MK_API __attribute__((visibility("default")))
#  elif defined _MSC_VER
#    define MK_API __declspec(dllexport)
#  else
#    error "No MK_BUILD_DLL action for compiler"
#  endif

#elif defined MK_USE_DLL

#  if defined __GNUC__
#    define MK_API /* Nothing. */
#  elif defined _MSC_VER
#    define MK_API __declspec(dllimport)
#  else
#    error "No MK_USE_DLL action for compiler"
#  endif

#else
#  define MK_API /* Nothing. */
#endif

// clang-format on

#endif
