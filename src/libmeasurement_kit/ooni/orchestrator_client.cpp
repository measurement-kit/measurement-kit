// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ext.hpp>
#include <measurement_kit/ooni.hpp>
#include <measurement_kit/http.hpp>

namespace mk {
namespace ooni {
namespace orchestratorx {


void Authentication::maybe_login(Callback<Error> cb,
                                 Settings settings,
                                 Var<Reactor> reactor, Var<Logger> logger) {
  if (is_valid()) {
    logger->debug("orchestrator: token is valid, no need to login");
    cb(NoError());
    return;
  }
  logger->debug("orchestrator: logging in");
  login(cb, settings, reactor, logger);
  return;
}

void Authentication::login(Callback<Error> cb,
                           Settings settings,
                           Var<Reactor> reactor, Var<Logger> logger) {

  nlohmann::json request;
  request["username"] = username;
  request["password"] = password;

  settings["http/url"] = base_url + "/api/v1/login";
  settings["http/method"] = "POST";
  logger->debug("orchestrator: sending login request");
  http::request(settings, {{"Content-Type", "application/json"}},
               request.dump(),
               [=](Error error, Var<http::Response> resp) {
                  if (error) {
                    logger->warn("orchestrator: error in logging in");
                    cb(error);
                    return;
                  }
                  try {
                    logger->debug("orchestrator: parsing login response");
                    auto response = nlohmann::json::parse(resp->body);
                    if (response.find("error") != response.end()) {
                      if (response["error"] == "wrong-username-password") {
                        cb(RegistryWrongUsernamePasswordError());
                        return;
                      }
                      if (response["error"] == "missing-username-password") {
                        cb(RegistryMissingUsernamePasswordError());
                        return;
                      }
                    }
                    logger->debug("orchestrator: parsing time %s", response["expire"].get<std::string>().c_str());
                    error = parse_iso8601_utc(response["expire"],
                                              &expiry_time);
                    if (error) {
                      cb(error);
                      return;
                    }
                    logger->debug("orchestrator: setting token");
                    token_ = response["token"];
                    logger->debug("orchestrator: token set");
                    logged_in_ = true;
                    cb(NoError());
                    return;
                  } catch (std::invalid_argument &) {
                    cb(JsonParseError());
                    return;
                  } catch (std::out_of_range &) {
                    cb(JsonKeyError());
                    return;
                  } catch (std::domain_error &) {
                    cb(JsonDomainError());
                    return;
                  }
               }, reactor, logger, nullptr, 0);
}

const std::string Authentication::get_token() {
  return token_;
}

bool Authentication::is_valid() {
  struct tm now;
  if (logged_in_ == false) {
    return false;
  }
  utc_time_now(&now);
  if (difftime(mktime(&now), mktime(&expiry_time)) <= 0) {
    return false;
  }
  return true;
}

void Authentication::refresh(Callback<Error> cb,
                             Settings settings,
                             Var<Reactor> reactor, Var<Logger> logger) {
  // XXX not implemented.
  cb(NoError());
}

void http_request_json(std::string url, std::string method,
                       std::string data,
                       http::Headers headers,
                       Callback<Error, Var<http::Response>, nlohmann::json> cb,
                       Settings settings,
                       Var<Reactor> reactor,
                       Var<Logger> logger) {
  settings["http/url"] = url;
  settings["http/method"] = method;
  headers["Content-Type"] = "application/json";
  logger->debug("orchestrator: posting to %s", url.c_str());
  logger->debug("   data: \"%s\"", data.c_str());
  http::request(settings, headers,
               data,
               [=](Error error, Var<http::Response> response) {
                  nlohmann::json jresponse;
                  if (error) {
                    cb(error, response, jresponse);
                    return;
                  }
                  try {
                    jresponse = nlohmann::json::parse(response->body);
                    cb(NoError(), response, jresponse);
                    return;
                  } catch (std::invalid_argument &) {
                    cb(JsonParseError(), response, jresponse);
                    return;
                  } catch (std::out_of_range &) {
                    cb(JsonKeyError(), response, jresponse);
                    return;
                  } catch (std::domain_error &) {
                    cb(JsonDomainError(), response, jresponse);
                    return;
                  }
               }, reactor, logger, nullptr, 0);
}

void http_request_json(std::string url, std::string method,
                       http::Headers headers,
                       Callback<Error, Var<http::Response>, nlohmann::json> cb,
                       Settings settings,
                       Var<Reactor> reactor,
                       Var<Logger> logger) {
  std::string data;
  http_request_json(url, method, data, headers, cb, settings, reactor, logger);
}

void http_request_json(std::string url, std::string method,
                       nlohmann::json jdata,
                       http::Headers headers,
                       Callback<Error, Var<http::Response>, nlohmann::json> cb,
                       Settings settings,
                       Var<Reactor> reactor,
                       Var<Logger> logger) {
  http_request_json(url, method, jdata.dump(), headers, cb, settings, reactor, logger);
}

nlohmann::json ProbeData::get_json() {
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

void ProbeData::register_probe(std::string registry_url,
                               std::string password,
                               Callback<Error, std::string> cb,
                               Settings settings,
                               Var<Reactor> reactor,
                               Var<Logger> logger) {
  if (probe_cc.empty() || probe_asn.empty() || platform.empty() ||
      software_name.empty() || software_version.empty()) {
    cb(MissingRequiredValueError(), "");
    return;
  }
  if ((platform == "ios" || platform == "android") && token.empty()) {
    cb(MissingRequiredValueError(), "");
    return;
  }

  nlohmann::json request = get_json();
  request["password"] = password;

  http_request_json(registry_url + "/api/v1/register", "POST", request, {},
      [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
    std::string client_id;
    if (error) {
      cb(error, "");
      return;
    }
    if (jresp.find("error") != jresp.end()) {
      if (jresp["error"] == "invalid request") {
        cb(RegistryInvalidRequestError(), "");
        return;
      }
    }
    client_id = jresp["client_id"].get<std::string>();
    if (client_id == "") {
      // XXX return a different error
      cb(RegistryInvalidRequestError(), "");
      return;
    }
    cb(NoError(), client_id);
  }, settings, reactor, logger);
}

void ProbeData::update(std::string registry_url,
                       Var<Authentication> auth,
                       Callback<Error> cb,
                       Settings settings,
                       Var<Reactor> reactor,
                       Var<Logger> logger) {
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err);
      return;
    }
    nlohmann::json request = get_json();

    http_request_json(registry_url + "/api/v1/update/" + auth->username, "PUT", request,
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      if (error) {
        cb(error);
        return;
      }
      // XXX add better error handling
      if (jresp.find("error") != jresp.end()) {
        logger->warn("orchestrator: update failed with \"%s\"", jresp["error"].get<std::string>().c_str());
        cb(RegistryInvalidRequestError());
        return;
      }
      if (jresp.find("status") == jresp.end() || jresp["status"] != "ok") {
        cb(RegistryInvalidRequestError());
        return;
      }
      cb(NoError());
    }, settings, reactor, logger);

  }, settings, reactor, logger);
}

void Task::get(Callback<Error, nlohmann::json> cb,
               Settings settings,
               Var<Reactor> reactor,
               Var<Logger> logger) {
  // XXX I may want to make this into it's own struct
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err, {});
      return;
    }

    http_request_json(events_url + "/api/v1/task/" + task_id, "GET",
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      if (error) {
        cb(error, {});
        return;
      }
      // XXX add better error handling
      cb(NoError(), jresp);
    }, settings, reactor, logger);
  }, settings, reactor, logger);
}

void Task::accept(Callback<Error> cb,
                  Settings settings,
                  Var<Reactor> reactor,
                  Var<Logger> logger) {
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err);
      return;
    }
    http_request_json(events_url + "/api/v1/task/" + task_id + "/accept",
        "POST",
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      if (error) {
        cb(error);
        return;
      }
      // XXX add better error handling
      cb(NoError());
    }, settings, reactor, logger);
  }, settings, reactor, logger);
}

void Task::reject(Callback<Error> cb,
                  Settings settings,
                  Var<Reactor> reactor,
                  Var<Logger> logger) {
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err);
      return;
    }
    http_request_json(events_url + "/api/v1/task/" + task_id + "/reject",
        "POST",
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      if (error) {
        cb(error);
        return;
      }
      // XXX add better error handling
      cb(NoError());
    }, settings, reactor, logger);
  }, settings, reactor, logger);
}

void Task::done(Callback<Error> cb,
                Settings settings,
                Var<Reactor> reactor,
                Var<Logger> logger) {
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err);
      return;
    }
    http_request_json(events_url + "/api/v1/task/" + task_id + "/done",
        "POST",
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      if (error) {
        cb(error);
        return;
      }
      // XXX add better error handling
      cb(NoError());
    }, settings, reactor, logger);
  }, settings, reactor, logger);
}

void list_tasks(std::string base_url,
                Var<Authentication> auth,
                Callback<Error, std::vector<Task>> cb,
                Settings settings,
                Var<Reactor> reactor,
                Var<Logger> logger) {
  auth->maybe_login([=](Error err){
    if (err) {
      cb(err, {});
      return;
    }
    http_request_json(base_url + "/api/v1/tasks", "GET",
        {{"Authorization", "Bearer " + auth->get_token()}},
        [=](Error error, Var<http::Response> resp, nlohmann::json jresp) {
      std::vector<Task> task_list;
      if (error) {
        cb(error, {});
        return;
      }
      // XXX add better error handling
      for (auto task : jresp["tasks"]) {
        std::string tid = task["id"].get<std::string>();
        Task t(auth, tid, base_url);
        task_list.push_back(t);
      }
      cb(NoError(), task_list);
    }, settings, reactor, logger);
  }, settings, reactor, logger);
}

std::string production_registry_url() {
    return MK_OONI_PRODUCTION_PROTEUS_REGISTRY_URL;
}

std::string testing_registry_url() {
    return MK_OONI_TESTING_PROTEUS_REGISTRY_URL;
}

std::string production_events_url() {
    return MK_OONI_PRODUCTION_PROTEUS_EVENTS_URL;
}

std::string testing_events_url() {
    return MK_OONI_TESTING_PROTEUS_EVENTS_URL;
}


} // namespace orchestratorx
} // namespace ooni
} // namespace mk
