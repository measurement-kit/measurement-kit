// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKMOCK_HPP
#define MEASUREMENT_KIT_MKMOCK_HPP

/// @file mkmock.hpp
///
/// This file contains common macros used for testing and mocking.

#include <exception>
#include <mutex>
#include <utility>

/// MKMOCK_HOOK_DISABLED is a disabled hook for @p Tag and @p Variable.
#define MKMOCK_HOOK_DISABLED(Tag, Variable)  // Nothing

/// MKMOCK_HOOK_ALLOC_DISABLED is like MKMOCK_HOOK_DISABLED but with
/// an additional @p Deleter to avoid memory leaks.
#define MKMOCK_HOOK_ALLOC_DISABLED(Tag, Variable, Deleter)  // Nothing

/// MKMOCK_HOOK_ENABLED provides you a hook to override the value of @p
/// Variable and identified by the @p Tag unique tag. Use this macro like:
///
/// ```
/// int rv = call_some_api();
/// MKMOCK_HOOK_ENABLED(call_some_api, rv);
/// if (rv != 0) {
///   // Handle error
///   return;
/// }
/// ````
#define MKMOCK_HOOK_ENABLED(Tag, Variable)                 \
  do {                                                     \
    mkmock_##Tag *inst = mkmock_##Tag::singleton();        \
    std::unique_lock<std::recursive_mutex> _{inst->mutex}; \
    if (inst->enabled) {                                   \
      Variable = inst->value;                              \
    }                                                      \
  } while (0)

/// MKMOCK_HOOK_ALLOC_ENABLED is like MKMOCK_HOOK_ENABLED except that it
/// uses a @p Deleter to be called to free allocated memory when we want to
/// make a successful memory allocation look like a failure. Without
/// using this macro, `asan` will complain about a memory leak.
#define MKMOCK_HOOK_ALLOC_ENABLED(Tag, Variable, Deleter)  \
  do {                                                     \
    mkmock_##Tag *inst = mkmock_##Tag::singleton();        \
    std::unique_lock<std::recursive_mutex> _{inst->mutex}; \
    if (inst->enabled) {                                   \
      if (Variable != nullptr) {                           \
        Deleter(Variable);                                 \
      }                                                    \
      Variable = inst->value;                              \
    }                                                      \
  } while (0)

/// MKMOCK_DEFINE_HOOK defines a hook with tag @p Tag and type @p Type. This
/// macro should be used in the unit tests source file only.
#define MKMOCK_DEFINE_HOOK(Tag, Type)  \
  class mkmock_##Tag {                 \
   public:                             \
    static mkmock_##Tag *singleton() { \
      static mkmock_##Tag instance;    \
      return &instance;                \
    }                                  \
                                       \
    bool enabled = false;              \
    Type value = {};                   \
    Type saved_value = {};             \
    std::exception_ptr saved_exc;      \
    std::recursive_mutex mutex;        \
  }

/// MKMOCK_WITH_ENABLED_HOOK runs @p CodeSnippet with the mock identified by
/// @p Tag enabled and with its value set to @p MockedValue. When leaving this
/// macro will disable the mock and set its value back to the old value, even
/// when @p CodeSnippet throws an exception. Exceptions will be rethrown by
/// this macro once the previous state has been reset.
#define MKMOCK_WITH_ENABLED_HOOK(Tag, MockedValue, CodeSnippet)   \
  /* Implementation note: this macro is written such that it   */ \
  /* can call itself without triggering compiler warning about */ \
  /* reusing the same names in a inner scope.                  */ \
  do {                                                            \
    {                                                             \
      mkmock_##Tag *inst = mkmock_##Tag::singleton();             \
      inst->mutex.lock(); /* Barrier for other threads */         \
      inst->saved_exc = {};                                       \
      inst->saved_value = inst->value;                            \
      inst->value = MockedValue;                                  \
      inst->enabled = true;                                       \
    }                                                             \
    try {                                                         \
      CodeSnippet                                                 \
    } catch (...) {                                               \
      mkmock_##Tag *inst = mkmock_##Tag::singleton();             \
      inst->saved_exc = std::current_exception();                 \
    }                                                             \
    {                                                             \
      mkmock_##Tag *inst = mkmock_##Tag::singleton();             \
      inst->enabled = false;                                      \
      inst->value = inst->saved_value;                            \
      inst->saved_value = {};                                     \
      std::exception_ptr saved_exc;                               \
      std::swap(saved_exc, inst->saved_exc);                      \
      inst->mutex.unlock(); /* Allow another thread. */           \
      if (saved_exc) {                                            \
        std::rethrow_exception(saved_exc);                        \
      }                                                           \
    }                                                             \
  } while (0)

#endif  // MEASUREMENT_KIT_MKMOCK_HPP
