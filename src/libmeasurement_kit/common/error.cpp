// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

namespace mk {

ErrorContext::~ErrorContext() {}

nlohmann::json ErrorContext::as_json() const {
    return nullptr;
}

nlohmann::json Error::as_json() const {
    nlohmann::json ctx_repr;
    if (context) {
        ctx_repr = context->as_json();
    }
    nlohmann::json frame{
        as_ooni_error(),
        ctx_repr
    };
    for (auto e : child_errors) {
        frame.push_back(e->as_json());
    }
    return frame;
}

} // namespace mk
