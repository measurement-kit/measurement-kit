// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_OONI_UTILS_HPP
#define PRIVATE_OONI_UTILS_HPP

#include <measurement_kit/ooni.hpp>

#include <GeoIP.h>
#include <GeoIPCity.h>

using json = nlohmann::json;

namespace mk {
namespace ooni {

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
    static Var<GeoipCache> thread_local_instance();

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

    // This means we cache up to 3 GeoIP database handles, based on filepath and
    // expire them in FIFO order.
    size_t max_size = 3;

  private:
    std::map<std::string, Var<GeoipDatabase>> instances;
};

std::string extract_html_title(std::string body);

bool is_private_ipv4_addr(const std::string &ipv4_addr);

std::string scrub(
        std::string orig,
        std::string real_probe_ip
);

} // namespace ooni
} // namespace mk
#endif
