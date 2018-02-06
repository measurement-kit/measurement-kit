// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define MK_ENGINE_INTERNALS // need to access internals

#include <measurement_kit/swig.hpp>

#include <memory>
#include <string>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/engine.h>

namespace mk {
namespace swig {

Task::Task() {}

bool Task::initialize(const std::string &settings) {
    if (pimpl_ != nullptr) {
        return false;
    }
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(settings);
    } catch (const std::exception &exc) {
        mk::warn("cannot parse json: %s", exc.what());
        return false;
    }
    pimpl_ = std::make_unique<engine::Task>(std::move(json));
    return true;
}

std::string Task::wait_for_next_event() {
    return std::string{
            (pimpl_) ? pimpl_->wait_for_next_event().dump() : "null"};
}

void Task::interrupt() {
    if (pimpl_ != nullptr) {
        pimpl_->interrupt();
    }
}

Task::~Task() {}

} // namespace swig
} // namespace mk
