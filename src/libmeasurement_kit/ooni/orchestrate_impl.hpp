// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_ORCHESTRATE_IMPL_HPP

#include <measurement_kit/http.hpp>

namespace mk {
namespace ooni {
namespace orchestrate {

// Everything related to authentication. Not suitable to be used in a
// multithreaded context. Provides the guarantee that you can safely
// forget about this object even when there are pending I/O operations.
class Authentication {
  public:
    std::string username;
    std::string password;
    std::string base_url;
    std::tm expiry_time = {};

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
        return (ofile.good()) ? NoError() : FileIoError();
    }

    const std::string get_token() const noexcept { return *token_; }

    bool is_valid() const noexcept {
        std::tm now = {};
        if (*logged_in_ == false) {
            return false;
        }
        utc_time_now(&now);
        if (difftime(mktime(&now), mktime(&expiry_time)) <= 0) {
            return false;
        }
        return true;
    }

    void maybe_login(Callback<Error> &&cb, Settings settings = {},
                     Var<Reactor> reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global()) {
        if (is_valid()) {
            logger->debug("orchestrator: token is valid, no need to login");
            cb(NoError());
            return;
        }
        logger->debug("orchestrator: logging in");
        login(std::move(cb), settings, reactor, logger);
    }

    template <MK_MOCK_AS(http::request_json_object, http_request_json_object)>
    void login(Callback<Error> &&cb, Settings settings = {},
               Var<Reactor> reactor = Reactor::global(),
               Var<Logger> logger = Logger::global()) {
        nlohmann::json request{{"username", username}, {"password", password}};
        logger->info("orchestrator: sending login request");
        /*
         * Important: do not pass `this` to the lambda closure. Rather make
         * sure everything we pass can be kept safe by the closure.
         */
        http_request_json_object(
              "POST", base_url + "/api/v1/login", request, {},
              [
                token = this->token_, logged_in = this->logged_in_,
                cb = std::move(cb)
              ](Error error, Var<http::Response> /*http_response*/,
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
                      if ((error = parse_iso8601_utc(ts, &expiry_time))) {
                          throw error;
                      }
                  });
                  if (!error) {
                      logger->info("orchestrator: logged in");
                      *token = response["token"];
                      *logged_in = true;
                  } else {
                      logger->warn("orchestrator: json processing error: %s",
                                   error.explain().c_str());
                  }
                  cb(error);
              },
              settings, reactor, logger);
    }

    void refresh(Callback<Error> &&cb, Settings /*settings*/ = {},
                 Var<Reactor> /*reactor*/ = Reactor::global(),
                 Var<Logger> /*logger*/ = Logger::global()) {
        // XXX not implemented.
        cb(NoError());
    }

  private:
    Var<bool> logged_in_ = Var<bool>::make(false);
    Var<std::string> token_ = Var<std::string>::make();
};

class ProbeData {
  public:
    // XXX: these seems to duplicate a thingie or two in the API
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

    nlohmann::json get_json() const {
        nlohmann::json j;
        j["probe_cc"] = probe_cc;
        j["probe_asn"] = probe_asn;
        j["platform"] = platform;
        j["software_name"] = software_name;
        j["software_version"] = software_version;
        if (!supported_tests.empty()) {
            j["supported_tests"] = supported_tests;
        }
        if (!network_type.empty()) {
            j["network_type"] = network_type;
        }
        if (!available_bandwidth.empty()) {
            j["available_bandwidth"] = available_bandwidth;
        }
        if (!token.empty()) {
            j["token"] = token;
        }
        if (!probe_family.empty()) {
            j["probe_family"] = probe_family;
        }
        return j;
    }

    template <MK_MOCK(http_request_json_object)>
    void register_probe(std::string registry_url, std::string password,
                        Callback<Error, std::string> &&cb,
                        Settings settings = {},
                        Var<Reactor> reactor = Reactor::global(),
                        Var<Logger> logger = Logger::global()) {
        if (probe_cc.empty() || probe_asn.empty() || platform.empty() ||
            software_name.empty() || software_version.empty()) {
            // FIXME: also the call soon must be done carefully
            cb(MissingRequiredValueError(), "");
            return;
        }
        if ((platform == "ios" || platform == "android") && token.empty()) {
            cb(MissingRequiredValueError(), "");
            return;
        }

        nlohmann::json request = get_json();
        request["password"] = password;

        http_request_json_object(
              "POST", registry_url + "/api/v1/register", request, {},
              [cb = std::move(cb)](Error error, Var<http::Response> /*resp*/,
                                   nlohmann::json json_response) {
                  std::string client_id;
                  if (error) {
                      cb(error, client_id);
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
                            client_id = jresp["client_id"];
                            if (client_id == "") {
                                // XXX return a different error
                                throw RegistryInvalidRequestError();
                            }
                        });
                  cb(error, client_id);
              },
              settings, reactor, logger);
    }

    void update(std::string registry_url, Var<Authentication> auth,
                Callback<Error> &&cb, Settings settings = {},
                Var<Reactor> reactor = Reactor::global(),
                Var<Logger> logger = Logger::global()) {
        //
    }
};

} // namespace orchestrate
} // namespace ooni
} // namespace mk
#endif
