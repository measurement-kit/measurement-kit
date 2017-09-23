// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include "private/ooni/orchestrate_impl.hpp"
#include "private/common/worker.hpp"

namespace mk {
namespace ooni {
namespace orchestrate {

/*
 * URLs
 */

std::string production_registry_url() {
    return MK_OONI_PRODUCTION_PROTEUS_REGISTRY_URL;
}

std::string testing_registry_url() {
    return MK_OONI_TESTING_PROTEUS_REGISTRY_URL;
}

std::string production_events_url() {
    return MK_OONI_PRODUCTION_PROTEUS_EVENTS_URL;
}

std::string testing_events_url() { return MK_OONI_TESTING_PROTEUS_EVENTS_URL; }

/*
 * Auth
 */

/*static*/ std::string Auth::make_password() {
    return mk::random_printable(64);
}

Error Auth::load(const std::string &filepath) noexcept {
    ErrorOr<std::string> maybe_data = slurp(filepath);
    if (!maybe_data) {
        return maybe_data.as_error();
    }
    return loads(*maybe_data);
}

Error Auth::loads(const std::string &s) noexcept {
    return json_process(s, [&](auto json) {
        auth_token = json.at("auth_token");
        expiry_time = json.at("expiry_time");
        logged_in = json.at("logged_in");
        username = json.at("username");
        password = json.at("password");
    });
}

Error Auth::dump(const std::string &filepath) noexcept {
    return overwrite_file(filepath, dumps());
}

std::string Auth::dumps() noexcept {
    auto json = Json{{"auth_token", auth_token},
                               {"expiry_time", expiry_time},
                               {"logged_in", logged_in},
                               {"username", username},
                               {"password", password}};
    return json.dump(4);
}

bool Auth::is_valid(SharedPtr<Logger> logger) const noexcept {
    if (!logged_in) {
        logger->debug("orchestrator: not logged in");
        return false;
    }
    if (auth_token.empty()) {
        logger->warn("orchestrator: auth_token is empty");
        return false;
    }

    tm expiry_temp{};
    logger->debug("orchestrator: expiry_time: '%s'", expiry_time.c_str());
    Error error = parse_iso8601_utc(expiry_time, &expiry_temp);
    if (error) {
        logger->warn("orchestrator: cannot parse expiry_time");
        return false;
    }
    auto expiry_time_s = std::mktime(&expiry_temp);
    if (expiry_time_s == (time_t)-1) {
        logger->warn("orchestrator: std::mktime() failed");
        return false;
    }
    logger->debug("orchestrator: expiry_time_s: %llu",
                  (unsigned long long)expiry_time_s);

    auto now_localtime = std::time(nullptr);
    if (now_localtime == (time_t)-1) {
        logger->warn("orchestrator: std::time() failed");
        return false;
    }
    logger->debug("orchestrator: now_localtime: %llu",
                  (unsigned long long)now_localtime);
    tm now_temp{};
    if (gmtime_r(&now_localtime, &now_temp) == nullptr) {
        logger->warn("orchestrator: std::gmtime_r() failed");
        return false;
    }
    auto now_utc = std::mktime(&now_temp);
    if (now_utc == (time_t)-1) {
        logger->warn("orchestrator: std::mktime() failed");
        return false;
    }
    logger->debug("orchestrator: now_utc: %llu",
                  (unsigned long long)now_utc);

    auto diff = difftime(expiry_time_s, now_utc);
    if (diff < 0) {
        logger->debug("orchestrator: the auth_token is expired");
        return false;
    }
    return true;
}

/*
 * Registry database
 */

Json ClientMetadata::as_json() const {
    Json j;
    // Keep the following sorted to ease comparison with class definition
    if (!available_bandwidth.empty()) {
        j["available_bandwidth"] = available_bandwidth;
    }
    if (!device_token.empty()) {
        j["token"] = device_token;
    }
    if (!language.empty()) {
        j["language"] = language;
    }
    if (!network_type.empty()) {
        j["network_type"] = network_type;
    }
    j["platform"] = platform;
    j["probe_asn"] = probe_asn;
    j["probe_cc"] = probe_cc;
    if (!probe_family.empty()) {
        j["probe_family"] = probe_family;
    }
    j["software_name"] = software_name;
    j["software_version"] = software_version;
    if (!supported_tests.empty()) {
        j["supported_tests"] = supported_tests;
    }
    return j;
}

void Client::register_probe(std::string &&password,
                            Callback<Error &&, Auth &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    //
    // We must copy or move everything into the initial lambda and then from
    // there the task is synchronous because run_...() is blocking.
    Worker::default_tasks_queue()->call_in_thread([
        password = std::move(password), meta = *this, cb = std::move(cb)
    ]() mutable {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([&]() {
            do_register_probe(std::move(password), meta, reactor,
                              [&](Error &&error, Auth &&auth) {
                                  reactor->stop();
                                  cb(std::move(error), std::move(auth));
                              });
        });
    });
}

void Client::find_location(
      Callback<Error &&, std::string &&, std::string &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    //
    // We must copy or move everything into the initial lambda and then from
    // there the task is synchronous because run_...() is blocking.
    Worker::default_tasks_queue()->call_in_thread(
          [ meta = *this, cb = std::move(cb) ]() {
              SharedPtr<Reactor> reactor = Reactor::make();
              reactor->run_with_initial_event([&]() {
                  do_find_location(meta, reactor,
                                   [&](Error &&error, std::string &&asn,
                                       std::string &&cc) {
                                       reactor->stop();
                                       cb(std::move(error), std::move(asn),
                                          std::move(cc));
                                   });
              });
          });
}

void Client::update(Auth &&auth, Callback<Error &&, Auth &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    //
    // We must copy or move everything into the initial lambda and then from
    // there the task is synchronous because run_...() is blocking.
    Worker::default_tasks_queue()->call_in_thread([
        meta = *this, cb = std::move(cb), auth = std::move(auth)
    ]() mutable {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([&]() {
            do_update(std::move(auth), meta, reactor,
                      [&](Error &&error, Auth &&auth) {
                          reactor->stop();
                          cb(std::move(error), std::move(auth));
                      });
        });
    });
}

void Client::list_tasks(Auth &&,
                        Callback<Error &&, Auth &&, std::vector<Task> &&>
                              && /*callback)*/) const {
    throw NotImplementedError();
}

/*
 * Events database
 */

void Task::get(
      Auth &&,
      Callback<Error &&, Auth &&, std::string &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::accept(Auth &&, Callback<Error &&, Auth &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::reject(Auth &&, Callback<Error &&, Auth &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::done(Auth &&, Callback<Error &&, Auth &&> && /*callback*/) const {
    throw NotImplementedError();
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
