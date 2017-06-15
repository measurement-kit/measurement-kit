// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

NameServers::NameServers() {}

NameServers::NameServers(std::nullptr_t) {}

#if 0
NameServers::NameServers(std::string name_server) {
    name_servers_.push_back(name_server);
}
#endif

NameServers::NameServers(std::initializer_list<std::string> name_servers)
    : name_servers_(name_servers) {}

void NameServers::push_back(std::string name_server) {
    name_servers_.push_back(name_server);
}

const std::list<std::string> &NameServers::as_list() const noexcept {
    return name_servers_;
}

} // namespace dns
} // namespace mk
