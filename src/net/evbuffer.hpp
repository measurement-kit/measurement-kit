// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_EVBUFFER_HPP
#define SRC_NET_EVBUFFER_HPP

#include <measurement_kit/common.hpp>
#include <functional>
#include <new>
#include "src/common/libs_impl.hpp"
struct evbuffer;

namespace mk {
namespace net {

/// RAII wrapper for evbuffer
class Evbuffer : public NonCopyable, public NonMovable {
  public:
    /// Constructor with optional libs pointer
    Evbuffer(Libs *libs = get_global_libs()) : libs_(libs) {}

    /// Destructor
    ~Evbuffer() {
        if (evbuf_ != nullptr) {
            libs_->evbuffer_free(evbuf_);
        }
    }

    /// Cast to evbuffer-* operator
    operator evbuffer *() {
        if (evbuf_ == nullptr && (evbuf_ = libs_->evbuffer_new()) == nullptr) {
            throw std::bad_alloc();
        }
        return evbuf_;
    }

    /// Access the underlying libevent
    Libs *get_libs() { return libs_; }

  private:
    Libs *libs_ = get_global_libs();
    evbuffer *evbuf_ = nullptr;
};

} // namespace net
} // namespace mk
#endif
