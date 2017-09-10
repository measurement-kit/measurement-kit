// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_UTILS_HPP
#define MEASUREMENT_KIT_OONI_UTILS_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               Reactor reactor = Reactor::global(),
               Var<Logger> logger = Logger::global());

void resolver_lookup(Callback<Error, std::string> callback, Settings = {},
                     Reactor reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global());

report::Entry represent_string(const std::string &s);

} // namespace ooni
} // namespace mk
#endif
