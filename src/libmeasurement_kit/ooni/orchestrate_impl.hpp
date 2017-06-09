// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP

#include <measurement_kit/http.hpp>

#include "../common/utils.hpp"

namespace mk {
namespace ooni {
namespace orchestrate {

class Authentication {
  public:
    std::string auth_token;
    std::tm expiry_time = {};
    bool logged_in = false;
    std::string username;
    std::string password;

    Error load(const std::string &filepath) {
        ErrorOr<std::string> maybe_data = slurp(filepath);
        if (!maybe_data) {
            return maybe_data.as_error();
        }
        return json_parse_process_and_filter_errors(
              *maybe_data, [&](auto json) {
                  username = json.at("username");
                  password = json.at("password");
              });
    }

    Error store(const std::string &filepath) {
        // TODO: we can perhaps make a better work at mapping file errors
        std::ofstream ofile{filepath};
        if (!ofile.good()) {
            return FileIoError();
        }
        nlohmann::json serio{{"username", username}, {"password", password}};
        ofile << serio.dump(4) << "\n";
        if (!ofile.good()) {
            return FileIoError();
        }
        return NoError();
    }

    bool is_valid() noexcept {
        std::tm now = {};
        if (logged_in == false) {
            return false;
        }
        utc_time_now(&now);
        if (difftime(mktime(&now), mktime(&expiry_time)) <= 0) {
            return false;
        }
        return true;
    }
};

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void login(Var<Authentication> auth, std::string registry_url,
           Settings settings, Var<Reactor> reactor, Var<Logger> logger,
           Callback<Error> &&cb) {
    if (auth->username == "" || auth->password == "") {
        logger->warn("orchestrator: missing username or password");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([cb = std::move(cb)]() {
            cb(MissingRequiredValueError());
        });
        return;
    }
    nlohmann::json request{{"username", auth->username},
                           {"password", auth->password}};
    logger->info("orchestrator: sending login request: %s",
                 request.dump().c_str());
    /*
     * Important: do not pass `this` to the lambda closure. Rather make
     * sure everything we pass can be kept safe by the closure.
     */
    http_request_json_object(
          "POST", registry_url + "/api/v1/login", request, {},
          [ auth, cb = std::move(cb),
            logger ](Error error, Var<http::Response> /*http_response*/,
                     nlohmann::json json_response) {
              if (error) {
                  logger->warn("orchestrator: JSON API error: %s",
                               error.explain().c_str());
                  cb(error);
                  return;
              }
              logger->debug("orchestrator: processing login response");
              error = json_process(json_response, [&](auto response) {
                  if (response.find("error") != response.end()) {
                      if (response["error"] == "wrong-username-password") {
                          throw RegistryWrongUsernamePasswordError();
                      }
                      if (response["error"] == "missing-username-password") {
                          throw RegistryMissingUsernamePasswordError();
                      }
                      // Note: this is basically an error case that we did
                      // not anticipate when writing the code
                      throw GenericError();
                  }
                  std::string ts = response["expire"];
                  logger->debug("orchestrator: parsing time %s", ts.c_str());
                  if ((error = parse_iso8601_utc(ts, &auth->expiry_time))) {
                      throw error;
                  }
                  // FIXME: I believe the token name here is too generic
                  auth->auth_token = response["token"];
                  auth->logged_in = true;
                  logger->info("orchestrator: logged in");
              });
              if (error) {
                  logger->warn("orchestrator: json processing error: %s",
                               error.explain().c_str());
              }
              cb(error);
          },
          settings, reactor, logger);
}

static inline void maybe_login(Var<Authentication> auth,
                               std::string registry_url, Settings settings,
                               Var<Reactor> reactor, Var<Logger> logger,
                               Callback<Error> &&cb) {
    if (auth->is_valid()) {
        logger->debug("orchestrator: auth token is valid, no need to login");
        cb(NoError());
        return;
    }
    logger->debug("orchestrator: logging in");
    login(auth, registry_url, settings, reactor, logger, std::move(cb));
}

static inline void refresh(Var<Authentication> /*auth*/, Settings /*settings*/,
                           Var<Reactor> /*reactor*/, Var<Logger> /*logger*/,
                           Callback<Error> &&cb) {
    // XXX not implemented.
    cb(NoError());
}

static inline nlohmann::json as_json(const ClientMetadata &m) {
    nlohmann::json j;
    j["probe_cc"] = m.probe_cc;
    j["probe_asn"] = m.probe_asn;
    j["platform"] = m.platform;
    j["software_name"] = m.software_name;
    j["software_version"] = m.software_version;
    if (!m.supported_tests.empty()) {
        j["supported_tests"] = m.supported_tests;
    }
    if (!m.network_type.empty()) {
        j["network_type"] = m.network_type;
    }
    if (!m.available_bandwidth.empty()) {
        j["available_bandwidth"] = m.available_bandwidth;
    }
    if (!m.device_token.empty()) {
        // FIXME: I think the "token" name is too generic
        j["token"] = m.device_token;
    }
    if (!m.probe_family.empty()) {
        j["probe_family"] = m.probe_family;
    }
    return j;
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void register_probe_(const ClientMetadata &m, std::string password,
                     Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger,
                     Callback<Error, Var<Authentication>> &&cb) {

    Var<Authentication> auth = Var<Authentication>::make();
    auth->password = password;

    if (m.probe_cc.empty() || m.probe_asn.empty() || m.platform.empty() ||
        m.software_name.empty() || m.software_version.empty() ||
        m.supported_tests.empty()) {
        logger->warn("orchestrator: missing required value");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([ cb = std::move(cb), auth ]() {
            cb(MissingRequiredValueError(), auth);
        });
        return;
    }
    if ((m.platform == "ios" || m.platform == "android") &&
        m.device_token.empty()) {
        logger->warn("orchestrator: you passed me an empty device token");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([ cb = std::move(cb), auth ]() {
            cb(MissingRequiredValueError(), auth);
        });
        return;
    }

    nlohmann::json request = as_json(m);
    request["password"] = password;

    http_request_json_object(
          "POST", m.registry_url + "/api/v1/register", request, {},
          [ cb = std::move(cb), logger, auth ](Error error,
                                               Var<http::Response> /*resp*/,
                                               nlohmann::json json_response) {
              if (error) {
                  logger->warn("orchestrator: JSON API error: %s",
                               error.explain().c_str());
                  cb(error, auth);
                  return;
              }
              error = json_process_and_filter_errors(
                    json_response, [&](auto jresp) {
                        if (jresp.find("error") != jresp.end()) {
                            if (jresp["error"] == "invalid request") {
                                throw RegistryInvalidRequestError();
                            }
                            // A case that we have not anticipated
                            throw GenericError();
                        }
                        auth->username = jresp["client_id"];
                        if (auth->username == "") {
                            throw RegistryEmptyClientIdError();
                        }
                    });
              if (error) {
                  logger->warn("orchestrator: JSON processing error: %s",
                               error.explain().c_str());
              }
              cb(error, auth);
          },
          settings, reactor, logger);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void update_(const ClientMetadata &m, Var<Authentication> auth,
             Settings settings, Var<Reactor> reactor, Var<Logger> logger,
             Callback<Error> &&cb) {
    std::string update_url =
          m.registry_url + "/api/v1/update/" + auth->username;
    nlohmann::json update_request = as_json(m);
    maybe_login(auth, m.registry_url, settings, reactor, logger, [
        update_url = std::move(update_url),
        update_request = std::move(update_request), cb = std::move(cb), auth,
        settings, reactor, logger
    ](Error err) {
        if (err != NoError()) {
            // Note: error printed by Authentication
            cb(err);
            return;
        }
        http_request_json_object(
              "PUT", update_url, update_request,
              {{"Authorization", "Bearer " + auth->auth_token}},
              [ cb = std::move(cb), logger ](Error err,
                                             Var<http::Response> /*resp*/,
                                             nlohmann::json json_response) {
                  if (err) {
                      // Note: error printed by Authentication
                      cb(err);
                      return;
                  }
                  err = json_process(json_response, [&](auto jresp) {
                      // XXX add better error handling
                      if (jresp.find("error") != jresp.end()) {
                          std::string s = jresp["error"];
                          logger->warn("orchestrator: update "
                                       "failed with \"%s\"",
                                       s.c_str());
                          throw RegistryInvalidRequestError();
                      }
                      if (jresp.find("status") == jresp.end() ||
                          jresp["status"] != "ok") {
                          throw RegistryInvalidRequestError();
                      }
                  });
                  cb(err);
              },
              settings, reactor, logger);
    });
}

static inline std::string make_secrets_path(std::string dir) noexcept {
#if (defined _WIN32 || defined __CYGWIN__)
    dir += R"xx(\)xx";
#else
    dir += "/";
#endif
    dir += "orchestrator_secrets.json";
    return dir;
}

static inline ErrorOr<Var<Authentication>> load_auth(std::string working_dir) {
    std::string fpath = make_secrets_path(working_dir);
    Var<Authentication> auth = Var<Authentication>::make();
    Error err = auth->load(fpath);
    if (err) {
        return err;
    }
    return auth;
}

static inline std::string make_password() {
    return mk::random_printable(64);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void do_register_probe(const ClientMetadata &m, std::string password,
                       Settings settings, Var<Reactor> reactor,
                       Var<Logger> logger, Callback<Error> &&cb) {
    ErrorOr<Var<Authentication>> ma = load_auth(m.working_dir);
    if (!!ma) {
        // Assume that, if we can load the secrets, we are already registered
        logger->info("This probe is already registered");
        reactor->call_soon([=]() { cb(NoError()); });
        return;
    }
    std::string destpath = make_secrets_path(m.working_dir);
    register_probe_<http_request_json_object>(
          m, password, settings, reactor, logger,
          [
            cb = std::move(cb), destpath = std::move(destpath)
          ](Error err, Var<Authentication> auth) {
              if (err) {
                  cb(std::move(err));
                  return;
              }
              cb(auth->store(destpath));
          });
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void do_update(const ClientMetadata &m, Settings settings, Var<Reactor> reactor,
               Var<Logger> logger, Callback<Error &&> &&cb) {
    ErrorOr<Var<Authentication>> ma = load_auth(m.working_dir);
    if (!ma) {
        reactor->call_soon([=]() { cb(ma.as_error()); });
        return;
    }
    update_<http_request_json_object>(m, *ma, settings, reactor, logger,
                                      std::move(cb));
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
