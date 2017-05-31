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

class RegisterProbe : public ClientData {

};

void Client::register_probe(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::register_probe", logger,
                                 RegisterProbe{*this}, std::move(callback));
}

void Client::update(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::update", logger, Update{*this},
                                 std::move(callback));
}

void Client::list_tasks(Callback<Error &&, std::vector<Task> &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::list_tasks", logger,
                                 ListTasks{*this}, std::move(callback));
}

/*
 * Events database
 */

// XXX: The task probably needs to know about the Client...

void Task::get(Callback<Error &&, std::string &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::task_get", logger,
                                 TaskGet{*this}, std::move(callback));
}

void Task::accept(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::task_accept", logger,
                                 TaskAccept{*this}, std::move(callback));
}

void Task::reject(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::task_reject", logger,
                                 TaskReject{*this}, std::move(callback));
}

void Task::done(Callback<Error &&> &&callback) {
    AsyncRunner::global()->start("orchestrate::task_done", logger,
                                 TaskDone{*this}, std::move(callback));
}

} // namespace orchestrate
} // namespace ooni
} // namespace mk
