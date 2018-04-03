// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP

#include <measurement_kit/ooni.hpp>
#include "src/libmeasurement_kit/report/entry.hpp"

#include <GeoIP.h>
#include <GeoIPCity.h>

namespace mk {
namespace ooni {

class GeoipDatabase {
  public:
    GeoipDatabase(std::string path) : path(path) {}

    ErrorOr<std::string>
    resolve_country_code(std::string ip, SharedPtr<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_country_name(std::string ip, SharedPtr<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_city_name(std::string ip, SharedPtr<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_asn(std::string ip, SharedPtr<Logger> = Logger::global());

    std::string path;

  private:
    ErrorOr<std::string>
    with_open_database_do(std::function<ErrorOr<std::string>()> action,
                          SharedPtr<Logger> logger);

    SharedPtr<GeoIP> db;
};

class GeoipCache {
  public:
    static SharedPtr<GeoipCache> thread_local_instance();

    SharedPtr<GeoipDatabase> get(std::string path, bool &did_open);

    SharedPtr<GeoipDatabase> get(std::string path) {
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
                         SharedPtr<Logger> logger = Logger::global()) {
        logger->debug("resolve country code '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_country_code(ip, logger);
    }

    ErrorOr<std::string>
    resolve_country_name(std::string path, std::string ip,
                         SharedPtr<Logger> logger = Logger::global()) {
        logger->debug("resolve country name '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_country_name(ip, logger);
    }

    ErrorOr<std::string>
    resolve_city_name(std::string path, std::string ip,
                      SharedPtr<Logger> logger = Logger::global()) {
        logger->debug("resolve city code '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_city_name(ip, logger);
    }

    ErrorOr<std::string>
    resolve_asn(std::string path, std::string ip,
                SharedPtr<Logger> logger = Logger::global()) {
        logger->debug("resolve asn '%s' using '%s'",
                      ip.c_str(), path.c_str());
        return get(path)->resolve_asn(ip, logger);
    }

    // This means we cache up to 3 GeoIP database handles, based on filepath and
    // expire them in FIFO order.
    size_t max_size = 3;

  private:
    std::map<std::string, SharedPtr<GeoipDatabase>> instances;
};

std::string extract_html_title(std::string body);

bool is_private_ipv4_addr(const std::string &ipv4_addr);

std::string scrub(
        std::string orig,
        std::string real_probe_ip
);

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               SharedPtr<Reactor> reactor = Reactor::global(),
               SharedPtr<Logger> logger = Logger::global());

void resolver_lookup(Callback<Error, std::string> callback, Settings = {},
                     SharedPtr<Reactor> reactor = Reactor::global(),
                     SharedPtr<Logger> logger = Logger::global());

report::Entry represent_string(const std::string &s);

} // namespace ooni
} // namespace mk
#endif
