// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_CAPTIVE_PORTAL_HPP
#define SRC_LIBMEASUREMENT_CAPTIVE_PORTAL_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

typedef std::map<std::string,std::string> input_t;
void gen_http_inputs(Var<std::vector<input_t>> is, Var<Logger> logger);

} // namespace ooni
} // namespace mk

#endif
