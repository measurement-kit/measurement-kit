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
 * Registry database
 */

nlohmann::json ClientMetadata::as_json_() const {
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

void Client::register_probe(Callback<Error &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start("orchestrate::register_probe", logger, [
        meta = *this, cb = std::move(cb)
    ](Continuation<> && done) {
        do_register_probe(meta, AsyncRunner::global()->reactor(), [
            done = std::move(done), cb = std::move(cb)
        ](Error && error) {
            done([ error = std::move(error), cb = std::move(cb) ]() mutable {
                cb(std::move(error));
            });
        });
    });
}

void Client::find_location(
      Callback<Error &&, std::string &&, std::string &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start(
          "orchestrate::find_location", logger,
          [ meta = *this, cb = std::move(cb) ](Continuation<> && done) {
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

void Client::update(Callback<Error &&> &&cb) const {
    // Copy the data contained by this object so we completely detach the
    // destiny of `this` and of the callback.
    AsyncRunner::global()->start("orchestrate::update", logger, [
        meta = *this, cb = std::move(cb)
    ](Continuation<> && done) {
        do_update(meta, AsyncRunner::global()->reactor(), [
            cb = std::move(cb), done = std::move(done)
        ](Error && error) {
            done([ cb = std::move(cb), error = std::move(error) ]() mutable {
                cb(std::move(error));
            });
        });
    });
}

void Client::list_tasks(
      Callback<Error &&, std::vector<Task> &&> && /*callback)*/) const {
    throw NotImplementedError();
}

/*
 * Events database
 */

void Task::get(Callback<Error &&, std::string &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::accept(Callback<Error &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::reject(Callback<Error &&> && /*callback*/) const {
    throw NotImplementedError();
}

void Task::done(Callback<Error &&> && /*callback*/) const {
    throw NotImplementedError();
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
