// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_ORCHESTRATE_HPP
#define MEASUREMENT_KIT_OONI_ORCHESTRATE_HPP

// Documentation: doc/api/ooni/orchestrate.md

#include <measurement_kit/common.hpp>

namespace mk {
namespace ooni {
namespace orchestrate {

/*
 * URLs
 */

#define MK_OONI_PRODUCTION_PROTEUS_REGISTRY_URL                                \
    "https://a.registry.proteus.ooni.io"
#define MK_OONI_TESTING_PROTEUS_REGISTRY_URL                                   \
    "https://registry.proteus.test.ooni.io"
#define MK_OONI_PRODUCTION_PROTEUS_EVENTS_URL "https://a.events.proteus.ooni.io"
#define MK_OONI_TESTING_PROTEUS_EVENTS_URL "https://events.proteus.test.ooni.io"

std::string production_registry_url();
std::string testing_registry_url();

std::string production_events_url();
std::string testing_events_url();

/*
 * Registry database
 */

class ClientMetadata {
  public:
    Var<Logger> logger = Logger::global();
    Settings settings = {};
    std::string available_bandwidth;
    std::string device_token;
    std::string events_url = production_events_url();
    std::string language;
    std::string network_type;
    std::string geoip_country_path;
    std::string geoip_asn_path;
    std::string platform;
    std::string probe_asn;
    std::string probe_cc;
    std::string probe_family;
    std::string registry_url = production_registry_url();
    std::string secrets_path = "orchestrator_secrets.json";
    std::string software_name = "measurement_kit";
    std::string software_version = MK_VERSION;
    std::vector<std::string> supported_tests;
    nlohmann::json as_json_() const;
};

class Task; /* Forward declaration */

class Client : public ClientMetadata {
  public:
    void register_probe(Callback<Error &&> &&callback) const;
    void update(Callback<Error &&> &&callback) const;
    void list_tasks(Callback<Error &&, std::vector<Task> &&> &&callback) const;
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
    void get(Callback<Error &&, std::string &&> &&callback) const;
    void accept(Callback<Error &&> &&callback) const;
    void reject(Callback<Error &&> &&callback) const;
    void done(Callback<Error &&> &&callback) const;
};

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
