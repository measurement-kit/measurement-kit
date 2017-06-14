// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP

#include <measurement_kit/ooni.hpp>

#include "../common/utils.hpp"
#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {
namespace orchestrate {

class Authentication : public HasMakeFactory<Authentication> {
  public:
    std::string auth_token;
    std::time_t expiry_time = {};
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
        return overwrite_file(filepath, nlohmann::json{{"username", username},
                                                       {"password", password}}
                                              .dump(4));
    }

    bool is_valid() const noexcept {
        // Assume that `std::time()` is not going to fail. Accoring to macOS
        // manpage it can fail when `gettimeofday` can fail. In turn, the latter
        // can fail with EFAULT (invalid buffer, not applicable here).
        return logged_in && difftime(expiry_time, std::time(nullptr)) >= 0;
    }
};

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void login(Var<Authentication> auth, std::string registry_url,
           Settings settings, Var<Reactor> reactor, Var<Logger> logger,
           Callback<Error &&> &&cb) {
    if (auth->username == "" || auth->password == "") {
        logger->warn("orchestrator: missing username or password");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([cb = std::move(cb)]() {
            cb(MissingRequiredValueError());
        });
        return;
    };
    nlohmann::json request{{"username", auth->username},
                           {"password", auth->password}};
    logger->info("Logging you in with orchestrator");
    logger->debug("orchestrator: sending login request: %s",
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
                  cb(std::move(error));
                  return;
              }
              logger->debug("orchestrator: processing login response");
              error = json_process_and_filter_errors(
                    json_response, [&](auto response) {
                        if (response.find("error") != response.end()) {
                            if (response["error"] ==
                                "wrong-username-password") {
                                throw RegistryWrongUsernamePasswordError();
                            }
                            if (response["error"] ==
                                "missing-username-password") {
                                throw RegistryMissingUsernamePasswordError();
                            }
                            // Note: this is basically an error case that we did
                            // not anticipate when writing the code
                            throw GenericError();
                        }
                        std::string ts = response["expire"];
                        logger->debug("orchestrator: parsing time %s",
                                      ts.c_str());
                        if ((error =
                                   parse_iso8601_utc(ts, &auth->expiry_time))) {
                            throw error;
                        }
                        auth->auth_token = response["token"];
                        auth->logged_in = true;
                        logger->info("Logged in with orchestrator");
                    });
              if (error) {
                  logger->warn("orchestrator: json processing error: %s",
                               error.explain().c_str());
              }
              cb(std::move(error));
          },
          settings, reactor, logger);
}

static inline void maybe_login(Var<Authentication> auth,
                               std::string registry_url, Settings settings,
                               Var<Reactor> reactor, Var<Logger> logger,
                               Callback<Error> &&cb) {
    if (auth->is_valid()) {
        logger->debug("orchestrator: auth token is valid, no need to login");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([cb = std::move(cb)]() {
            cb(NoError());
        });
        return;
    }
    logger->debug("orchestrator: logging in");
    login(auth, registry_url, settings, reactor, logger, std::move(cb));
}

static inline void refresh(Var<Authentication> /*auth*/, Settings /*settings*/,
                           Var<Reactor> /*reactor*/, Var<Logger> /*logger*/,
                           Callback<Error> && /*cb*/) {
    throw NotImplementedError();
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void register_probe_(const ClientMetadata &m, std::string password,
                     Var<Reactor> reactor,
                     Callback<Error, Var<Authentication>> &&cb) {

    Var<Authentication> auth = Authentication::make();
    auth->password = password;

    if (m.probe_cc.empty() || m.probe_asn.empty() || m.platform.empty() ||
        m.software_name.empty() || m.software_version.empty() ||
        m.supported_tests.empty()) {
        m.logger->warn("orchestrator: missing required value");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([ cb = std::move(cb), auth ]() {
            cb(MissingRequiredValueError(), auth);
        });
        return;
    }
    if ((m.platform == "ios" || m.platform == "android") &&
        m.device_token.empty()) {
        m.logger->warn("orchestrator: you passed me an empty device token");
        // Guarantee that the callback will not be called immediately
        reactor->call_soon([ cb = std::move(cb), auth ]() {
            cb(MissingRequiredValueError(), auth);
        });
        return;
    }

    nlohmann::json request = m.as_json_();
    request["password"] = password;

    http_request_json_object(
          "POST", m.registry_url + "/api/v1/register", request, {},
          [ cb = std::move(cb), logger = m.logger,
            auth ](Error error, Var<http::Response> /*resp*/,
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
          m.settings, reactor, m.logger);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void update_(const ClientMetadata &m, Var<Authentication> auth,
             Var<Reactor> reactor, Callback<Error> &&cb) {
    std::string update_url =
          m.registry_url + "/api/v1/update/" + auth->username;
    nlohmann::json update_request = m.as_json_();
    maybe_login(auth, m.registry_url, m.settings, reactor, m.logger, [
        update_url = std::move(update_url),
        update_request = std::move(update_request), cb = std::move(cb), auth,
        settings = m.settings, reactor, logger = m.logger
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
                  err = json_process_and_filter_errors(
                        json_response, [&](auto jresp) {
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

static inline ErrorOr<Var<Authentication>> load_auth(std::string fpath) {
    Var<Authentication> auth = Authentication::make();
    Error err = auth->load(fpath);
    if (err) {
        return err;
    }
    return auth;
}

static inline std::string make_password() { return mk::random_printable(64); }

template <MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void do_find_location(const ClientMetadata &m, Var<Reactor> reactor,
                      Callback<Error &&, std::string &&, std::string> &&cb) {
    ooni_ip_lookup(
          [
            logger = m.logger, geoip_asn_path = m.geoip_asn_path,
            geoip_country_path = m.geoip_country_path, cb = std::move(cb)
          ](Error error, std::string ip) mutable {
              if (error) {
                  cb(std::move(error), "", "");
                  return;
              }
              logger->info("Probe IP is: %s", ip.c_str());
              auto query_geoip = [&](const std::string &path, std::string &dest,
                                     const std::string &fallback,
                                     auto callable) {
                  // Using local database to avoid thread safety issues
                  ooni::GeoipDatabase db{path};
                  ErrorOr<std::string> x = callable(db, ip, logger);
                  if (!x) {
                      logger->warn("geoip failed for '%s': %s", ip.c_str(),
                                   x.as_error().as_ooni_error().c_str());
                      dest = fallback;
                      return;
                  }
                  logger->info("IP %s maps to %s", ip.c_str(), x->c_str());
                  dest = *x;
              };
              std::string probe_asn;
              std::string probe_cc;
              query_geoip(geoip_asn_path, probe_asn, "AS0",
                          [](auto db, auto ip, auto logger) {
                              return db.resolve_asn(ip, logger);
                          });
              query_geoip(geoip_country_path, probe_cc, "ZZ",
                          [](auto db, auto ip, auto logger) {
                              return db.resolve_country_code(ip, logger);
                          });
              cb(NoError(), std::move(probe_asn), std::move(probe_cc));
          },
          m.settings, reactor, m.logger);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void possibly_register_probe_(
      Error error, ClientMetadata m, Var<Reactor> reactor,
      Callback<Error, ClientMetadata, Var<Reactor>> cb) {
    if (error) {
        cb(error, m, reactor);
        return;
    }
    ErrorOr<Var<Authentication>> x = load_auth(m.secrets_path);
    if (!!x) {
        // Assume that, if we can load the secrets, we are already registered
        m.logger->debug("This probe is already registered");
        cb(NoError(), m, reactor);
        return;
    }
    m.logger->info("Registering this probe with the orchestrator registry");
    register_probe_<http_request_json_object>(
          m, make_password(), reactor,
          [cb, m, reactor](Error err, Var<Authentication> auth) {
              if (!err) {
                  err = auth->store(m.secrets_path);
              }
              cb(err, m, reactor);
          });
}

template <MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void retrieve_missing_metadata_(
      Error error, ClientMetadata m, Var<Reactor> reactor,
      Callback<Error, ClientMetadata, Var<Reactor>> cb) {
    if (error) {
        cb(error, m, reactor);
        return;
    }
    if (m.platform == "") {
        m.platform = mk_platform();
    }
    if (m.supported_tests.empty()) {
        m.supported_tests = {{"dns_injection"},
                             {"http_header_field_manipulation"},
                             {"http_invalid_request_line"},
                             {"meek_fronted_requests"},
                             {"ndt"},
                             {"tcp_connect"},
                             {"web_connectivity"}};
    }
    if (m.probe_asn != "" && m.probe_cc != "") {
        m.logger->debug("No need to guess ASN and/or CC");
        cb(NoError(), m, reactor);
        return;
    }
    m.logger->info("Looking up probe IP to guess ASN and/or CC");
    do_find_location<ooni_ip_lookup>(
          m, reactor,
          [m, reactor, cb](Error &&error, std::string &&asn,
                           std::string &&cc) mutable {
              m.probe_asn = asn;
              m.probe_cc = cc;
              cb(error, m, reactor);
          });
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void finally_update_metadata_(Error error, ClientMetadata m,
                              Var<Reactor> reactor, Callback<Error> cb) {
    if (error) {
        cb(error);
        return;
    }
    ErrorOr<Var<Authentication>> x = load_auth(m.secrets_path);
    if (!x) {
        reactor->call_soon([=]() { cb(x.as_error()); });
        return;
    }
    update_<http_request_json_object>(m, *x, reactor, std::move(cb));
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object),
          MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void do_register_probe(const ClientMetadata &m, Var<Reactor> reactor,
                       Callback<Error &&> &&cb) {
    mk::fcompose_async(retrieve_missing_metadata_<ooni_ip_lookup>,
                       possibly_register_probe_<http_request_json_object>,
                       [](Error error, ClientMetadata, Var<Reactor>,
                          Callback<Error> cb) { cb(error); })(NoError(), m,
                                                              reactor, cb);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object_1),
          MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup),
          MK_MOCK_AS(http::request_json_object, http_request_json_object_2)>
void do_update(const ClientMetadata &m, Var<Reactor> reactor,
               Callback<Error &&> &&cb) {
    mk::fcompose_async(
        retrieve_missing_metadata_<ooni_ip_lookup>,
        possibly_register_probe_<http_request_json_object_1>,
        finally_update_metadata_<http_request_json_object_2>
    )(NoError(), m, reactor, cb);
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
