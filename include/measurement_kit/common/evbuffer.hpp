// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_EVBUFFER_HPP
#define MEASUREMENT_KIT_COMMON_EVBUFFER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>
struct evbuffer;

namespace measurement_kit {
namespace common {

/// RAII wrapper for evbuffer
class Evbuffer : public NonCopyable, public NonMovable {
  public:
    /// Constructor with optional libs pointer
    Evbuffer(Libs *libs = get_global_libs()) : libs_(libs) {}

    /// Destructor
    ~Evbuffer();

    /// Cast to evbuffer-* operator
    operator evbuffer *();

    /// Access the underlying libevent
    Libs *get_libs() { return (libs_); }

  private:
    Libs *libs_ = get_global_libs();
    evbuffer *evbuf_ = nullptr;
};

} // namespace common
} // namespace measurement_kit
#endif
