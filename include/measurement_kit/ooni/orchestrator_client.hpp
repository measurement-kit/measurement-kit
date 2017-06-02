// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_ORCHESTRATOR_CLIENT_HPP
#define MEASUREMENT_KIT_OONI_ORCHESTRATOR_CLIENT_HPP

#include <measurement_kit/http.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace orchestratorx {

#define MK_OONI_PRODUCTION_PROTEUS_REGISTRY_URL "https://a.registry.proteus.ooni.io"
#define MK_OONI_TESTING_PROTEUS_REGISTRY_URL "https://a.registry.proteus.test.ooni.io"
#define MK_OONI_PRODUCTION_PROTEUS_EVENTS_URL "https://a.events.proteus.ooni.io"
#define MK_OONI_TESTING_PROTEUS_EVENTS_URL "https://a.events.proteus.test.ooni.io"

std::string production_registry_url();
std::string testing_registry_url();

std::string production_events_url();
std::string testing_events_url();


/* Registry related */

class Authentication {
  public:
    std::string username;
    std::string password;
    std::string base_url;
    std::tm expiry_time;

    const std::string get_token();
    bool is_valid();

    void maybe_login(Callback<Error> cb,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());
    void login(Callback<Error> cb,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());
    void refresh(Callback<Error> cb,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());

  private:
    bool logged_in_ = false;
    std::string token_;
};

class ProbeData {
  public:
    std::string probe_cc;
    std::string probe_asn;
    std::string platform;
    std::string software_name;
    std::string software_version;
    std::vector<std::string> supported_tests;
    std::string network_type;
    std::string available_bandwidth;
    std::string token;
    std::string probe_family;

    nlohmann::json get_json();

    void register_probe(std::string registry_url,
                        std::string password,
                        Callback<Error, std::string>,
                        Settings = {},
                        Var<Reactor> = Reactor::global(),
                        Var<Logger> = Logger::global());
    void update(std::string registry_url,
                Var<Authentication> auth,
                Callback<Error>,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());
};

/* Events database related */

class Task {
  public:
    Task(Var<Authentication> a, std::string tid, std::string u) : auth(a), task_id(tid), events_url(u) {}

    Var<Authentication> auth;
    std::string task_id;
    std::string events_url;

    void get(Callback<Error, nlohmann::json>,
             Settings = {},
             Var<Reactor> = Reactor::global(),
             Var<Logger> = Logger::global());
    void accept(Callback<Error>,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());
    void reject(Callback<Error>,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());
    void done(Callback<Error>,
              Settings = {},
              Var<Reactor> = Reactor::global(),
              Var<Logger> = Logger::global());
};

void list_tasks(std::string base_url,
                Var<Authentication> auth,
                Callback<Error, std::vector<Task>>,
                Settings = {},
                Var<Reactor> = Reactor::global(),
                Var<Logger> = Logger::global());

} // namespace orchestratorx
} // namespace ooni
} // namespace mk
#endif
