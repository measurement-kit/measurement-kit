// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_FFI_HPP
#define SRC_LIBMEASUREMENT_KIT_FFI_HPP

#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/ffi.h>

#include <string>

// mk_event_ is the internal representation of an event.
struct mk_event_ : public std::string {
    using std::string::string;
};

// mk_event_create_ is an internal method to create an event from a JSON. We
// do not export this method, but we need it to be internally accessible to
// run unit tests and make sure we copy with bugs. See also issue #1728.
mk_event_t *mk_event_create_(const nlohmann::json &json) noexcept;

#endif  // SRC_LIBMEASUREMENT_KIT_FFI_HPP
