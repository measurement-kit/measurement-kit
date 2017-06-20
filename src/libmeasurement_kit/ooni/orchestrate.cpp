// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include "../ooni/orchestrate_impl.hpp"

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
    return json_parse_process_and_filter_errors(s, [&](auto json) {
        username = json.at("username");
        password = json.at("password");
    });
}

Error Auth::dump(const std::string &filepath) noexcept {
    return overwrite_file(filepath, dumps());
}

std::string Auth::dumps() noexcept {
    auto json = nlohmann::json{{"username", username}, {"password", password}};
    return json.dump(4);
}

bool Auth::is_valid() const noexcept {
    // Assume that `std::time()` is not going to fail. According to macOS
    // manpage it can fail when `gettimeofday` can fail. In turn, the latter
    // can fail with EFAULT (invalid buffer, not applicable here).
    return logged_in && difftime(expiry_time, std::time(nullptr)) >= 0;
}

/*
 * Registry database
 */

nlohmann::json ClientMetadata::as_json() const {
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
    if (!device_token.empty()) {
        j["token"] = device_token;
    }
    if (!probe_family.empty()) {
        j["probe_family"] = probe_family;
    }
    return j;
}

void Client::register_probe(std::string &&password,
                            Callback<Error &&, Auth &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start_("orchestrate::register_probe", logger, [
        password = std::move(password), meta = *this, cb = std::move(cb)
    ](Continuation<> && done) mutable {
        do_register_probe(
              std::move(password), meta, AsyncRunner::global()->reactor(),
              [ done = std::move(done),
                cb = std::move(cb) ](Error && error, Auth && auth) mutable {
                  done([
                      error = std::move(error), cb = std::move(cb),
                      auth = std::move(auth)
                  ]() mutable { cb(std::move(error), std::move(auth)); });
              });
    });
}

void Client::find_location(
      Callback<Error &&, std::string &&, std::string &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start_(
          "orchestrate::find_location", logger,
          [ meta = *this, cb = std::move(cb) ](Continuation<> && done) mutable {
              do_find_location(meta, AsyncRunner::global()->reactor(), [
                  cb = std::move(cb), done = std::move(done)
              ](Error && error, std::string && asn, std::string && cc) {
                  done([
                      cb = std::move(cb), error = std::move(error),
                      asn = std::move(asn), cc = std::move(cc)
                  ]() mutable {
                      cb(std::move(error), std::move(asn), std::move(cc));
                  });
              });
          });
}

void Client::update(Auth &&auth, Callback<Error &&, Auth &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start_("orchestrate::update", logger, [
        meta = *this, cb = std::move(cb), auth = std::move(auth)
    ](Continuation<> && done) mutable {
        do_update(std::move(auth), meta, AsyncRunner::global()->reactor(),
                  [ cb = std::move(cb), done = std::move(done) ](
                        Error && error, Auth && auth) mutable {
                      done([
                          cb = std::move(cb), error = std::move(error),
                          auth = std::move(auth)
                      ]() mutable { cb(std::move(error), std::move(auth)); });
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
