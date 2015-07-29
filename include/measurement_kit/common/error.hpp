// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

namespace measurement_kit {
namespace common {

struct Error {
    int error = 0;

    Error(int e) { this->error = e; };
};

}}
#endif
