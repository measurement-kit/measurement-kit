// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_EVBUFFER_HPP
#define MEASUREMENT_KIT_COMMON_EVBUFFER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>

#include <functional>
#include <new>

struct evbuffer;

namespace measurement_kit {
namespace common {

/// RAII wrapper for evbuffer
class Evbuffer : public NonCopyable, public NonMovable {
  public:
    /// Constructor with optional libevent pointer
    Evbuffer(Libs *libs = Libs::global()) : libs_(libs) {}

    /// Destructor
    ~Evbuffer() {
        if (evbuf_ != nullptr) libs_->evbuffer_free(evbuf_);
    }

    /// Cast to evbuffer-* operator
    operator evbuffer *() {
        if (evbuf_ == nullptr && (evbuf_ = libs_->evbuffer_new()) == nullptr)
            throw std::bad_alloc();
        return (evbuf_);
    }

    /// Access the underlying libevent
    Libs *get_libevent() { return (libs_); }

  private:
    Libs *libs_ = Libs::global();
    evbuffer *evbuf_ = nullptr;
};

} // namespace common
} // namespace measurement_kit
#endif
