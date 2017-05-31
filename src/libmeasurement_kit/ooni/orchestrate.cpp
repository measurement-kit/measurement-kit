// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

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

void Client::register_probe(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::register_probe", logger,
                                 RegisterProbe{*this}, std::move(callback));
}

void Client::update(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::update", logger, Update{*this},
                                 std::move(callback));
}

void Client::list_tasks(Callback<Error &&, std::vector<Task> &&> &&callback) {
    throw NotImplementedError();
}

/*
 * Events database
 */

void Task::get(Callback<Error &&, std::string &&> &&callback) {
    throw NotImplementedError();
}

void Task::accept(Callback<Error &&> &&callback) {
    throw NotImplementedError();
}

void Task::reject(Callback<Error &&> &&callback) {
    throw NotImplementedError();
}

void Task::done(Callback<Error &&> &&callback) { throw NotImplementedError(); }

} // namespace orchestrate
} // namespace ooni
} // namespace mk
