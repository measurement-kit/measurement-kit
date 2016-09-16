// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_OONI_UTILS_HPP
#define SRC_OONI_UTILS_HPP

#include <GeoIP.h>
#include <GeoIPCity.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni/error.hpp>
#include <regex>
#include <string>

using json = nlohmann::json;

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               Var<Reactor> reactor = Reactor::global(),
               Var<Logger> logger = Logger::global());

void resolver_lookup(Callback<Error, std::string> callback, Settings = {},
                     Var<Reactor> reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global());

class GeoipDatabase {
  public:
    GeoipDatabase(std::string path) : path(path) {}

    ErrorOr<std::string>
    resolve_country_code(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_country_name(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_city_name(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_asn(std::string ip, Var<Logger> = Logger::global());

    std::string path;

  private:
    ErrorOr<std::string>
    with_open_database_do(std::function<ErrorOr<std::string>()> action,
                          Var<Logger> logger);

    Var<GeoIP> db;
};

class GeoipCache {
  public:
    static Var<GeoipCache> global();

    Var<GeoipDatabase> get(std::string path, bool &did_open);

    Var<GeoipDatabase> get(std::string path) {
        bool did_open = false;
        return get(path, did_open);
    }

    // After you update resources you want to invalidate the cache
    // such that new databases are loaded on next query
    void invalidate() {
        instances.clear();
    }

    ErrorOr<std::string>
    resolve_country_code(std::string path, std::string ip,
                         Var<Logger> logger = Logger::global()) {
        logger->debug("resolve country code '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_country_code(ip, logger);
    }

    ErrorOr<std::string>
    resolve_country_name(std::string path, std::string ip,
                         Var<Logger> logger = Logger::global()) {
        logger->debug("resolve country name '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_country_name(ip, logger);
    }

    ErrorOr<std::string>
    resolve_city_name(std::string path, std::string ip,
                      Var<Logger> logger = Logger::global()) {
        logger->debug("resolve city code '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_city_name(ip, logger);
    }

    ErrorOr<std::string>
    resolve_asn(std::string path, std::string ip,
                Var<Logger> logger = Logger::global()) {
        logger->debug("resolve asn '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_asn(ip, logger);
    }

    size_t max_size = 3;

  private:
    std::map<std::string, Var<GeoipDatabase>> instances;
};

std::string extract_html_title(std::string body);

bool is_private_ipv4_addr(const std::string &ipv4_addr);

// Returns true if it's a ipv4 or ipv6 ip address
bool is_ip_addr(const std::string &ip_addr);

} // namespace ooni
} // namespace mk
#endif
