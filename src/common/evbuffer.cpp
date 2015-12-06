// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/evbuffer.hpp>
#include <measurement_kit/common/libs.hpp>  // for Libs
#include <functional>                       // for function
#include <new>                              // for bad_alloc
#include "src/common/libs_impl.hpp"

namespace mk {

Evbuffer::~Evbuffer() {
    if (evbuf_ != nullptr) libs_->evbuffer_free(evbuf_);
}

Evbuffer::operator evbuffer *() {
    if (evbuf_ == nullptr && (evbuf_ = libs_->evbuffer_new()) == nullptr)
        throw std::bad_alloc();
    return (evbuf_);
}

} // namespace mk
