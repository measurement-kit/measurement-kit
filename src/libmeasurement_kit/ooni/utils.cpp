// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/reactor.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/utils_impl.hpp"
#include "src/libmeasurement_kit/regexp/regexp.hpp"

namespace mk {
namespace ooni {

void resolver_lookup(Callback<Error, std::string> callback, Settings settings,
                     SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    resolver_lookup_impl(callback, settings, reactor, logger);
}

void find_location(std::string geoip_country_path, std::string geoip_asn_path,
        Settings settings, SharedPtr<Logger> logger,
        Callback<Error &&, std::string &&, std::string &&> &&cb) {
    orchestrate::Client client;
    client.logger = logger;
    client.geoip_asn_path = geoip_asn_path;
    client.geoip_country_path = geoip_country_path;
    client.settings = settings;
    client.find_location(std::move(cb));
}

std::string extract_html_title(std::string body) {
  return regexp::html_extract_title(body);
}

bool is_private_ipv4_addr(const std::string &ipv4_addr) {
  return regexp::private_ipv4(ipv4_addr);
}

report::Entry represent_string(const std::string &s) {
    Error error = utf8_parse(s);
    if (error != NoError()) {
        return report::Entry{{"format", "base64"},
                              {"data", base64_encode(s)}};
    }
    return s;
}

std::string scrub(std::string s, std::string real_probe_ip) {
    size_t p = 0;
    while ((p = s.find(real_probe_ip, p)) != std::string::npos) {
        s = s.replace(p, real_probe_ip.size(), "[REDACTED]");
    }
    return s;
}

} // namespace ooni
} // namespace mk
