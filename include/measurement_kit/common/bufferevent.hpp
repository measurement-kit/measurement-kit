// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_BUFFEREVENT_HPP
#define MEASUREMENT_KIT_COMMON_BUFFEREVENT_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>

#include <event2/util.h> // for evutil_socket_t
#include <functional>    // for function
#include <new>           // for bad_alloc
#include <stdexcept>     // for runtime_error

struct bufferevent;
struct event_base;

namespace measurement_kit {
namespace common {

/// RAII wrapper for bufferevent socket
class Bufferevent : public NonCopyable, public NonMovable {
  public:
    /// Default constructor
    Bufferevent(Libs *libs = Libs::global()) : libs_(libs) {}

    /// Constructor with socket
    Bufferevent(event_base *base, evutil_socket_t fd, int options,
                Libs *libs = Libs::global()) {
        make(base, fd, options, libs);
    }

    /// Attach to event base and socket
    void make(event_base *base, evutil_socket_t fd, int options,
              Libs *libs = Libs::global()) {
        close();
        libs_ = libs;
        //bev_ = libs_->bufferevent_socket_new(base, fd, options);
        bev_ = bufferevent_socket_new(base, fd, options);
        if (bev_ == nullptr) throw std::bad_alloc();
    }

    /// Destructor
    ~Bufferevent() { close(); }

    /// Idempotent close function
    void close() {
        if (bev_ != nullptr) {
            libs_->bufferevent_free(bev_);
            bev_ = nullptr;
        }
    }

    /// Automatic cast to pointer to bufferevent
    operator bufferevent *() {
        if (bev_ == nullptr) throw std::runtime_error("nullptr bufferevent");
        return (bev_);
    }

    /// Access underlying libs
    Libs *get_libs() { return (libs_); }

  private:
    Libs *libs_ = Libs::global();
    bufferevent *bev_ = nullptr;
};

} // namespace common
} // namespace measurement_kit
#endif
