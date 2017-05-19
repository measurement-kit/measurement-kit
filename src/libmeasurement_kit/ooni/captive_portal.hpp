// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_CAPTIVE_PORTAL_HPP
#define SRC_LIBMEASUREMENT_CAPTIVE_PORTAL_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

typedef std::map<std::string,std::string> input;
void gen_http_inputs(Var<std::vector<input>> is, Var<Logger> logger);

typedef std::function<bool(Var<http::Response>)> no_cp_f_t;
void gen_no_cp_f(const input &i, Var<no_cp_f_t> no_cp_f);

} // namespace ooni
} // namespace mk

#endif
