// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP

#include "src/libmeasurement_kit/common/fcompose.hpp"
#include <measurement_kit/common/json.hpp>
#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"

#include <measurement_kit/ooni.hpp>
#include "src/libmeasurement_kit/http/http.hpp"

#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {
namespace orchestrate {

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void login(Auth &&auth, std::string registry_url, Settings settings,
           SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
           Callback<Error &&, Auth &&> &&cb) {
    if (auth.username.empty() || auth.password.empty()) {
        logger->warn("orchestrator: missing username or password");
        cb(MissingRequiredValueError(), std::move(auth));
        return;
    };
    Json request{{"username", auth.username},
                           {"password", auth.password}};
    logger->info("Logging you in with orchestrator");
    logger->debug("orchestrator: sending login request: %s",
                  request.dump().c_str());
    /*
     * Important: do not pass `this` to the lambda closure. Rather make
     * sure everything we pass can be kept safe by the closure.
     */
    http_request_json_object(
          "POST", registry_url + "/api/v1/login", request, {},
          [ auth = std::move(auth), cb = std::move(cb),
            logger ](Error error, SharedPtr<http::Response> /*http_response*/,
                     Json json_response) mutable {
              if (error) {
                  logger->warn("orchestrator: JSON API error: %s",
                               error.what());
                  cb(std::move(error), std::move(auth));
                  return;
              }
              logger->debug("orchestrator: processing login response");
              error = json_process(
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
                        auth.expiry_time = response["expire"];
                        auth.auth_token = response["token"];
                        auth.logged_in = true;
                        logger->info("Logged in with orchestrator");
                    });
              if (error) {
                  logger->warn("orchestrator: json processing error: %s",
                               error.what());
              }
              cb(std::move(error), std::move(auth));
          },
          settings, reactor, logger);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void maybe_login(Auth &&auth, std::string registry_url, Settings settings,
                 SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
                 Callback<Error &&, Auth &&> &&cb) {
    if (auth.is_valid(logger)) {
        logger->info("Auth token is valid, no need to login");
        cb(NoError(), std::move(auth));
        return;
    }
    login<http_request_json_object>(std::move(auth), registry_url, settings,
                                    reactor, logger, std::move(cb));
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void register_probe_(const ClientMetadata &m, std::string password,
                     SharedPtr<Reactor> reactor, Callback<Error &&, Auth &&> &&cb) {
    Auth auth;
    auth.password = password;
    if (m.probe_cc.empty() || m.probe_asn.empty() || m.platform.empty() ||
        m.software_name.empty() || m.software_version.empty() ||
        m.supported_tests.empty()) {
        m.logger->warn("orchestrator: missing required value");
        cb(MissingRequiredValueError(), std::move(auth));
        return;
    }
    if ((m.platform == "ios" || m.platform == "android") &&
        m.device_token.empty()) {
        m.logger->warn("orchestrator: you passed me an empty device token");
        cb(MissingRequiredValueError(), std::move(auth));
        return;
    }
    Json request = m.as_json();
    request["password"] = password;
    http_request_json_object(
          "POST", m.registry_url + "/api/v1/register", request, {},
          [ cb = std::move(cb), logger = m.logger,
            auth = std::move(auth) ](Error error, SharedPtr<http::Response> /*resp*/,
                                     Json json_response) mutable {
              if (error) {
                  logger->warn("orchestrator: JSON API error: %s",
                               error.what());
                  cb(std::move(error), std::move(auth));
                  return;
              }
              error = json_process(
                    json_response, [&](auto jresp) {
                        if (jresp.find("error") != jresp.end()) {
                            if (jresp["error"] == "invalid request") {
                                throw RegistryInvalidRequestError();
                            }
                            // A case that we have not anticipated
                            throw GenericError();
                        }
                        auth.username = jresp["client_id"];
                        if (auth.username == "") {
                            throw RegistryEmptyClientIdError();
                        }
                    });
              if (error) {
                  logger->warn("orchestrator: JSON processing error: %s",
                               error.what());
              } else {
                  logger->info("Registered probe with orchestrator as: '%s'",
                               auth.username.c_str());
              }
              cb(std::move(error), std::move(auth));
          },
          m.settings, reactor, m.logger);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void update_(const ClientMetadata &m, Auth &&auth, SharedPtr<Reactor> reactor,
             Callback<Error &&, Auth &&> &&cb) {
    std::string update_url = m.registry_url + "/api/v1/update/" + auth.username;
    Json update_request = m.as_json();
    maybe_login(
          std::move(auth), m.registry_url, m.settings, reactor, m.logger, [
              update_url = std::move(update_url),
              update_request = std::move(update_request), cb = std::move(cb),
              settings = m.settings, reactor, logger = m.logger
          ](Error && err, Auth && auth) {
              if (err != NoError()) {
                  // Note: error printed by maybe_login()
                  cb(std::move(err), std::move(auth));
                  return;
              }
              // With GCC move inside lambda happens before setting headers, so
              // use a copy of the auth token to make sure we do not fail
              std::string auth_token = auth.auth_token;
              http_request_json_object(
                    "PUT", update_url, update_request,
                    {{"Authorization", "Bearer " + auth_token}},
                    [ cb = std::move(cb), logger, auth = std::move(auth) ](
                          Error err, SharedPtr<http::Response> /*resp*/,
                          Json json_response) mutable {
                        if (err) {
                            // Note: error printed by maybe_login()
                            cb(std::move(err), std::move(auth));
                            return;
                        }
                        err = json_process(
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
                        if (!err) {
                            logger->info("Updated orchestrator about "
                                         "this probe state");
                        }
                        cb(std::move(err), std::move(auth));
                    },
                    settings, reactor, logger);
          });
}

template <MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void do_find_location(const ClientMetadata &m, SharedPtr<Reactor> reactor,
                      Callback<Error &&, std::string &&, std::string &&> &&cb) {
    ooni_ip_lookup(
          [
            logger = m.logger, geoip_asn_path = m.geoip_asn_path,
            geoip_country_path = m.geoip_country_path, cb = std::move(cb)
          ](Error error, std::string ip) mutable {
              if (error) {
                  cb(std::move(error), "", "");
                  return;
              }
              logger->debug("Probe IP is: %s", ip.c_str());
              auto query_geoip = [&](const std::string &path, std::string &dest,
                                     const std::string &fallback,
                                     auto callable) {
                  // Using local database to avoid thread safety issues
                  ooni::GeoipDatabase db{path};
                  ErrorOr<std::string> x = callable(db, ip, logger);
                  if (!x) {
                      logger->warn("geoip failed for '%s': %s", ip.c_str(),
                                   x.as_error().what());
                      dest = fallback;
                      return;
                  }
                  logger->debug("IP %s maps to %s", ip.c_str(), x->c_str());
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

class RegistryCtx {
  public:
    Auth auth;
    SharedPtr<Logger> logger;
    ClientMetadata metadata;
    SharedPtr<Reactor> reactor;
};

static inline void ctx_enter_(Auth &&auth, ClientMetadata &&meta,
                              SharedPtr<Reactor> reactor,
                              Callback<SharedPtr<RegistryCtx>> &&cb) {
    auto ctx = SharedPtr<RegistryCtx>::make();
    ctx->auth = std::move(auth);
    ctx->metadata = std::move(meta);
    ctx->reactor = reactor;
    ctx->logger = ctx->metadata.logger;
    cb(ctx);
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void ctx_register_(Error &&error, SharedPtr<RegistryCtx> ctx,
                   Callback<Error &&, SharedPtr<RegistryCtx>> &&cb) {
    if (error) {
        cb(std::move(error), ctx);
        return;
    }
    auto p = ctx->auth.password.empty() ? Auth::make_password()
                                        : ctx->auth.password;
    ctx->logger->info("Registering this probe with the orchestrator registry");
    register_probe_<http_request_json_object>(
          ctx->metadata, p, ctx->reactor,
          [ cb = std::move(cb), ctx ](Error && err, Auth && auth) {
              ctx->auth = std::move(auth);
              cb(std::move(err), ctx);
          });
}

template <MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void ctx_retrieve_missing_meta_(SharedPtr<RegistryCtx> ctx,
                                Callback<Error &&, SharedPtr<RegistryCtx>> &&cb) {
    if (ctx->metadata.platform == "") {
        ctx->metadata.platform = mk_platform();
    }
    if (ctx->metadata.supported_tests.empty()) {
        ctx->metadata.supported_tests = {{"dns_injection"},
                                         {"http_header_field_manipulation"},
                                         {"http_invalid_request_line"},
                                         {"meek_fronted_requests"},
                                         {"ndt"},
                                         {"tcp_connect"},
                                         {"web_connectivity"}};
    }
    if (ctx->metadata.probe_asn != "" && ctx->metadata.probe_cc != "") {
        ctx->logger->debug("No need to guess ASN and/or CC");
        cb(NoError(), ctx);
        return;
    }
    ctx->logger->info("Looking up probe IP to guess ASN and/or CC");
    do_find_location<ooni_ip_lookup>(ctx->metadata, ctx->reactor, [
        cb = std::move(cb), ctx
    ](Error && error, std::string && asn, std::string && cc) mutable {
        ctx->metadata.probe_asn = std::move(asn);
        ctx->metadata.probe_cc = std::move(cc);
        cb(std::move(error), ctx);
    });
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
void ctx_update_(Error &&error, SharedPtr<RegistryCtx> ctx,
                 Callback<Error &&, SharedPtr<RegistryCtx>> &&cb) {
    if (error) {
        cb(std::move(error), ctx);
        return;
    }
    update_<http_request_json_object>(
          ctx->metadata, std::move(ctx->auth), ctx->reactor,
          [ cb = std::move(cb), ctx ](Error && error, Auth && auth) mutable {
              ctx->auth = std::move(auth);
              cb(std::move(error), ctx);
          });
}

static inline void ctx_leave_(Error &&error, SharedPtr<RegistryCtx> ctx,
                              Callback<Error &&, Auth &&> &&cb) {
    cb(std::move(error), std::move(ctx->auth));
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object),
          MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void do_register_probe(std::string &&password, const ClientMetadata &m,
                       SharedPtr<Reactor> reactor, Callback<Error &&, Auth &&> &&cb) {
    // For uniformity we pass an `Auth` structure to `ctx_enter_` and we
    // retrieve the password from it in `ctx_register_`
    Auth auth;
    auth.password = std::move(password);
    mk::fcompose(mk::fcompose_policy_async(), ctx_enter_,
                 ctx_retrieve_missing_meta_<ooni_ip_lookup>,
                 ctx_register_<http_request_json_object>,
                 ctx_leave_)(std::move(auth), m, reactor, std::move(cb));
}

template <MK_MOCK_AS(http::request_json_object, http_request_json_object),
          MK_MOCK_AS(ooni::ip_lookup, ooni_ip_lookup)>
void do_update(Auth &&auth, const ClientMetadata &m, SharedPtr<Reactor> reactor,
               Callback<Error &&, Auth &&> &&cb) {
    mk::fcompose(mk::fcompose_policy_async(), ctx_enter_,
                 ctx_retrieve_missing_meta_<ooni_ip_lookup>,
                 ctx_update_<http_request_json_object>,
                 ctx_leave_)(std::move(auth), m, reactor, std::move(cb));
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
