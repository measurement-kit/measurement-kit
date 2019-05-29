// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/dns/query.hpp"

namespace mk {
namespace ooni {

std::string extract_html_title(std::string body);

bool is_private_ipv4_addr(const std::string &ipv4_addr);

std::string scrub(
        std::string orig,
        std::string real_probe_ip
);

void resolver_lookup(Callback<Error, std::string> callback, Settings,
                     SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

nlohmann::json represent_string(const std::string &s);

std::string redact(Settings settings, std::string s);

} // namespace ooni
} // namespace mk
#endif
