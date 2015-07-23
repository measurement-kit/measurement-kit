// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_ERRORS_HPP
#define MEASUREMENT_KIT_OONI_ERRORS_HPP

#include <stdexcept>

namespace measurement_kit {
namespace ooni {

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InputFileRequired : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

}}
#endif
