// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_HPP

#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <measurement_kit/common/version.h>

#include "src/libmeasurement_kit/common/error.hpp"
#include "src/libmeasurement_kit/common/logger.hpp"
#include "src/libmeasurement_kit/common/settings.hpp"

namespace mk {
namespace ooni {
namespace orchestrate {

/*
 * URLs
 */

#define MK_OONI_PRODUCTION_PROTEUS_REGISTRY_URL                                \
    "https://ps1.ooni.io"
#define MK_OONI_TESTING_PROTEUS_REGISTRY_URL                                   \
    "https://ams-pg-test.ooni.org"
#define MK_OONI_PRODUCTION_PROTEUS_EVENTS_URL "https://ps1.ooni.io"
#define MK_OONI_TESTING_PROTEUS_EVENTS_URL "https://ams-pg-test.ooni.org"

std::string production_registry_url();
std::string testing_registry_url();

std::string production_events_url();
std::string testing_events_url();

/*
 * Registry database
 */

class Auth {
  public:
    std::string auth_token;
    std::string expiry_time;
    bool logged_in = false;
    std::string username;
    std::string password;

    static std::string make_password();
    Error load(const std::string &filepath) noexcept;
    Error loads(const std::string &data) noexcept;
    Error dump(const std::string &filepath) noexcept;
    std::string dumps() noexcept;
    bool is_valid(SharedPtr<Logger>) const noexcept;
};

class ClientMetadata {
  public:
    SharedPtr<Logger> logger = Logger::make();
    Settings settings = {};
    std::string available_bandwidth;
    std::string device_token;
    std::string ca_bundle_path;
    std::string events_url = production_events_url();
    std::string geoip_country_path;
    std::string geoip_asn_path;
    std::string language;
    std::string network_type;
    std::string platform;
    std::string probe_asn;
    std::string probe_cc;
    std::string probe_family;
    std::string probe_timezone;
    std::string registry_url = production_registry_url();
    std::string software_name = "measurement_kit";
    std::string software_version = MK_VERSION;
    std::vector<std::string> supported_tests;

    nlohmann::json as_json() const;
};

class Task; /* Forward declaration */

class Client : public ClientMetadata {
  public:
    void register_probe(
          std::string &&, Callback<Error &&, Auth &&> &&callback) const;

    void find_location(
          Callback<Error &&, std::string &&, std::string &&> &&callback) const;

    void update(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void list_tasks(
          Auth &&,
          Callback<Error &&, Auth &&, std::vector<Task> &&> &&callback) const;
};

/*
 * Events database
 */

class TaskData {
  public:
    std::string events_url = production_events_url();
    std::string task_id;
};

class Task : public TaskData {
  public:
    void get(Auth &&,
             Callback<Error &&, Auth &&, std::string &&> &&callback) const;

    void accept(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void reject(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void done(Auth &&, Callback<Error &&, Auth &&> &&callback) const;
};

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
