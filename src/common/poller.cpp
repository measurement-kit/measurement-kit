// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>

#include <stdexcept>

namespace measurement_kit {
namespace common {

Poller::Poller(Libs *libs) {
    if (libs != nullptr) libs_ = libs;
    if ((base_ = libs_->event_base_new()) == nullptr) throw std::bad_alloc();
    if ((dnsbase_ = libs_->evdns_base_new(base_, 1)) == nullptr) {
        libs_->event_base_free(base_);
        throw std::bad_alloc();
    }
}

Poller::~Poller() {
    libs_->evdns_base_free(dnsbase_, 1);
    libs_->event_base_free(base_);
}

void Poller::loop() {
    auto result = libs_->event_base_dispatch(base_);
    if (result < 0) throw std::runtime_error("event_base_dispatch() failed");
    if (result == 1) warn("loop: no pending and/or active events");
}

void Poller::break_loop() {
    if (libs_->event_base_loopbreak(base_) != 0)
        throw std::runtime_error("event_base_loopbreak() failed");
}

void Poller::loop_once() {
    auto result = libs_->event_base_loop(base_, EVLOOP_ONCE);
    if (result < 0) throw std::runtime_error("event_base_loop() failed");
    if (result == 1) warn("loop: no pending and/or active events");
}

} // namespace common
} // namespace measurement_kit
